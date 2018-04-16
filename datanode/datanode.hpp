/*
 *  Authors: Hongzhi Chen, Yunjian Zhao
 */

#ifndef DATANODE_HPP_
#define DATANODE_HPP_

#include <string.h>
#include <limits.h>
#include <map>
#include <unordered_map>
#include <set>
#include <stddef.h>

#include "util/mailbox.hpp"

using namespace std;

struct LargeFileInfo {
  std::vector<size_t> offsets;
  std::vector<size_t> indices;
  std::vector<size_t> buf_idx;
};

class Datanode{
public:
    Datanode(std::string root, int id){
        _root = root;
        _id = id;
    }
    void load_root_path();
    void init();
    void run();
    void loop();
    void mk_supdir(string path);
    void write_new_file(string filename, const char * buf, int size); //return the file descriptor id

private:
    std::string _root;
    int _id;
    MailBox _mailbox;

    set<string> files;
    set<string>::iterator fileIter;
    unordered_map<string, LargeFileInfo> large_file_index;
};


#endif /* DATANODE_HPP_ */
