/*
 *  Authors: Hongzhi Chen, Yunjian Zhao
 */

#include <string>
#include <string.h>
#include <assert.h>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include "datanode/datanode.hpp"
#include "util/global.hpp"
#include "util/msg.hpp"

using namespace std;

void Datanode::load_root_path(){
    string sid = to_string(_id);
    _root = _root + "/" + sid;
}

void Datanode::init(){
    check_dir(_root);
    //metadata update
}

void Datanode::mk_supdir(string path){
    const size_t last_c = path.find_last_of("/");
    if (string::npos != last_c)
    {
        path.erase(last_c, std::string::npos);
    }
    if(path.length()){
        path = _root + path;
        mk_dir(path.c_str());
    }
}

void Datanode::write_new_file(string path, const char * buf, int size)
{
    mk_supdir(path);

    char fname[PATH_MAX];
    strcpy(fname, _root.c_str());
    strcat(fname, path.c_str());

    FILE * file = fopen(fname, "wb");
    fwrite(buf, 1, size, file);
    fclose(file);
}

void Datanode::loop()
{
    while (true)
    {
        Command cmd = _mailbox.slaveBcastCMD();

        switch (cmd)
        {
            case Command::INIT:
            {
                init();
                worker_barrier();
                break;
            }

            case Command::S_WRITE:
            {
                SW_Meta meta;
                _mailbox.slaveBcast(meta);

                int write_len;
                string path(meta.path);
                fileIter = files.find(path);
                if(fileIter == files.end()){
                    //new file
                    assert(meta.offset == 0);
                    write_new_file(path, meta.buf, meta.size);

                    files.insert(path);
                    write_len = meta.size;
                }else{
                    string fname = _root + path;
                    int fd = open(fname.c_str(), O_RDWR);
                    write_len = pwrite(fd, meta.buf, meta.size, meta.offset);
                    close(fd);
                }
                int nid = get_next_nid();
                if(nid == get_worker_id()){
                    _mailbox.send(&write_len, sizeof(write_len), MASTER_RANK);
                }
                break;
            }

            case Command::L_WRITE:
            {
                LW_Meta meta;
                _mailbox.slaveScatter(meta);

                std::string dir = _root + meta.path;

                auto iter = large_file_index.find(meta.path);
                if(iter == large_file_index.end()){
                    //new file
                    // assert(0 == mkdir(dir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH));
                    write_new_file(meta.path, "", 0);
                    auto &info = large_file_index[meta.path];
                    info.offsets.push_back(0);
                    info.indices.push_back(0);
                    info.buf_idx.push_back(0);
                    // printf("Create new file %s\n", meta.path.c_str());
                } else if (meta.begin_offset ==0) {
                    auto &info = large_file_index[meta.path];
                    auto &offsets = info.offsets;
                    /* for (int i = 1 ; i < offsets.size(); i++) {
                        std::string file_name = dir + "/" + std::to_string(offsets[i - 1]) + "-" + std::to_string(offsets[i]);
                        assert(0 == remove(file_name.c_str()));
                    } */
                    offsets.resize(1);
                    info.indices.resize(1);
                    info.buf_idx.resize(1);
                    // write_new_file(dir, "", 0);
                } else {
                    assert(large_file_index[meta.path].offsets.back() == meta.begin_offset);
                }
                auto &info = large_file_index[meta.path];
                info.offsets.push_back(meta.end_offset);
                info.indices.push_back(meta.buf_index);
                // auto par = meta.path + "/" + std::to_string(meta.begin_offset) + "-" + std::to_string(meta.end_offset);
                char *c = (char*)meta.buf.c_str();
                // printf("%s\n", meta.buf.c_str());
                // write_new_file(par, c, meta.buf.length());
                // printf("%s\n",dir.c_str());
                int fd = open(dir.c_str(), O_RDWR);
                assert(fd >= 0);
                auto wlen = pwrite(fd, meta.buf.c_str(), meta.buf.length(), info.buf_idx.back());
                assert(meta.buf.length() == wlen);
                close(fd);
                info.buf_idx.push_back(info.buf_idx.back() + meta.buf.length());
                break;
            }

            case Command::S_READ:
            {
                int nid = get_next_nid();
                if(nid == get_worker_id()){

                    int wid = get_worker_id();
                    _mailbox.send(&wid, sizeof(wid), MASTER_RANK);

                    SR_Meta meta = _mailbox.recv_data<SR_Meta>(MASTER_RANK);
                    string path(meta.path);
                    fileIter = files.find(path);

                    if(fileIter == files.end()){
                        //new file
                        fprintf(stderr, "%s IS NOT FOUND\n", meta.path);
                        SR_PKG pkg(0, NULL, 0);
                        _mailbox.send_data(pkg, MASTER_RANK);

                    }else{
                        char buf[meta.size];

                        string fname = _root + path;
                        int fd = open(fname.c_str(), O_RDONLY);
                        int read_len = pread(fd, buf, meta.size, meta.offset);
                        close(fd);

                        SR_PKG pkg(1, buf, read_len);
                        _mailbox.send_data(pkg, MASTER_RANK);
                    }
                }
                break;
            }

            case Command::L_READ:
            {
                LR_Meta meta;
                _mailbox.slaveBcast(meta);
                assert(large_file_index.count(meta.path));

                auto &info = large_file_index[meta.path];
                auto &offsets = info.offsets;
                assert(offsets.back() > meta.begin_offset);

                int ed = std::lower_bound(offsets.begin(), offsets.end(), meta.begin_offset) - offsets.begin();
                if (offsets[ed] == meta.begin_offset) {
                    ed++;
                }

                int nid = get_next_nid();
                if(nid != get_worker_id()) {
                    // std::string fname = _root + meta.path + "/" + std::to_string(offsets[ed - 1]) + "-" + std::to_string(offsets[ed]);
                    std::string fname = _root + meta.path;
                    auto buf_size = info.buf_idx[ed] - info.buf_idx[ed - 1];
                    // auto buf_size = lseek(fd, 0, SEEK_END);
                    // lseek(fd, 0, SEEK_SET);
                    std::string buf(buf_size, 0);
                    char*c = (char*)buf.c_str();
                    int fd = open(fname.c_str(), O_RDONLY);
                    if (buf.size() != pread(fd, c, buf.size(), info.buf_idx[ed - 1])) {
                        printf("Failed to read to %s %d\n", fname.c_str(), (int)buf.size());
                        assert(false);
                    }
                    close(fd);
                    LR_PKG pkg(buf, offsets[ed - 1], offsets[ed], info.indices[ed]);
                    _mailbox.slaveGather(pkg);
                } else {
                    LR_PKG pkg("", offsets[ed - 1], offsets[ed], info.indices[ed]);
                    _mailbox.slaveGather(pkg);
                }
                break;
            }

            case Command::G_ATTR:
            {
                int nid = get_next_nid();
                if(nid == get_worker_id()){

                    int wid = get_worker_id();
                    _mailbox.send(&wid, sizeof(wid), MASTER_RANK);

                    ATTR_Meta meta = _mailbox.recv_data<ATTR_Meta>(MASTER_RANK);
                    string path(meta.path);
                    fileIter = files.find(path);

                    if(fileIter == files.end()){
                        if (large_file_index.count(path)) {
                            size_t size = large_file_index[path].offsets.back();
                            _mailbox.send(&size, sizeof(size_t), MASTER_RANK);
                        } else {
                            size_t EMPTY = 0;
                            _mailbox.send(&EMPTY, sizeof(size_t), MASTER_RANK);
                        }
                    }else{
                        string fname = _root + path;
                        struct stat statbuf;
                        stat(fname.c_str(), &statbuf);
                        size_t fsize = statbuf.st_size;

                        _mailbox.send(&fsize, sizeof(size_t), MASTER_RANK);
                    }
                }
                break;
            }
        }
    }
}



void Datanode::run(){
    load_root_path();
    std::cout << "DataNode " << get_worker_id() << " has started" << std::endl;
    loop();
}
