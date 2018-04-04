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

void Datanode::write_new_file(string path, char * buf, int size)
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
            	}
            	int nid = get_next_nid();
            	if(nid == get_worker_id()){
            		_mailbox.send(&write_len, sizeof(write_len), MASTER_RANK);
            	}
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

                		SR_PKG pkg(1, buf, read_len);
                		_mailbox.send_data(pkg, MASTER_RANK);
                	}
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
						int EMPTY = 0;
						_mailbox.send(&EMPTY, sizeof(int), MASTER_RANK);
					}else{
						string fname = _root + path;
						struct stat statbuf;
						stat(fname.c_str(), &statbuf);
						int fsize = statbuf.st_size;

						_mailbox.send(&fsize, sizeof(int), MASTER_RANK);
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
