/*
 *  Authors: Hongzhi Chen, Yunjian Zhao
 */

#include <assert.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>

#ifndef MSG_H_
#define MSG_H_


typedef struct SW_Meta{
public:
    char * path;
    int path_len;
    char * buf;
    size_t size;
    off_t offset;

    SW_Meta(){}

    SW_Meta (const char * _path, const char * _buf, size_t _size, off_t _offset){
        path_len = strlen(_path);
        path = new char[path_len+1];
        memcpy(path, _path, path_len+1);
        size = _size;
        offset = _offset;
        buf = new char[_size];
        memcpy(buf,_buf,_size);
    }

    ~SW_Meta(){
        delete [] path;
        delete [] buf;
    }

    friend ibinstream& operator<<(ibinstream& m, const SW_Meta& meta){
        m << meta.path_len;
        m.raw_bytes(meta.path, meta.path_len+1);
        m << meta.size;
        m << meta.offset;
        m.raw_bytes(meta.buf,meta.size);
        return m;
    }

    friend obinstream& operator>>(obinstream& m, SW_Meta& meta)
    {
        m >> meta.path_len;

        meta.path = new char[meta.path_len+1];
        char * data = (char*)m.raw_bytes(meta.path_len+1);
        memcpy(meta.path, data, meta.path_len+1);

        m >> meta.size;
        m >> meta.offset;

        meta.buf = new char[meta.size];
        data = (char*)m.raw_bytes(meta.size);
        memcpy(meta.buf, data, meta.size);

        return m;
    }
}SW_Meta;

typedef struct LW_Meta{
public:
    std::string path;
    std::string buf;
    int buf_index;
    size_t begin_offset, end_offset;


    LW_Meta(){}

    LW_Meta (std::string _path, std::string &&_data, int _buf_idx, size_t begin, size_t end){
        path = std::move(_path);
        buf = std::move(_data);
        buf_index = _buf_idx;
        begin_offset = begin;
        end_offset = end;
    }

    friend ibinstream& operator<<(ibinstream& m, const LW_Meta& meta){
        m << meta.path.length();
        m.raw_bytes(meta.path.c_str(), meta.path.length());
        m << meta.buf.length();
        m.raw_bytes(meta.buf.c_str(), meta.buf.length());
        m << meta.buf_index << meta.begin_offset << meta.end_offset;
        return m;
    }

    friend obinstream& operator>>(obinstream& m, LW_Meta& meta)
    {
        size_t len;

        m >> len;
        char * data = (char*)m.raw_bytes(len);
        meta.path = std::string(data, len);

        m >> len;
        data = (char*)m.raw_bytes(len);
        meta.buf = std::string(data, len);

        m >> meta.buf_index >> meta.begin_offset >> meta.end_offset;
        return m;
    }
}LW_Meta;

typedef struct SR_Meta{
public:
    char * path;
    int path_len;
    size_t size;
    off_t offset;

    SR_Meta(){}

    SR_Meta (const char * _path, size_t _size, off_t _offset){
        path_len = strlen(_path);
        path = new char[path_len+1];
        memcpy(path, _path, path_len+1);
        size = _size;
        offset = _offset;
    }

    ~SR_Meta(){
        delete [] path;
    }

    friend ibinstream& operator<<(ibinstream& m, const SR_Meta& meta){
        m << meta.path_len;
        m << meta.size;
        m << meta.offset;
        m.raw_bytes(meta.path, meta.path_len+1);
        return m;
    }

    friend obinstream& operator>>(obinstream& m, SR_Meta& meta)
    {
        m >> meta.path_len;
        m >> meta.size;
        m >> meta.offset;

        meta.path = new char[meta.path_len+1];
        char * data = (char*)m.raw_bytes(meta.path_len+1);
        memcpy(meta.path, data, meta.path_len+1);

        return m;
    }
}SR_Meta;

typedef struct LR_Meta{
public:
    std::string path;
    size_t begin_offset, end_offset;

    LR_Meta(){}

    LR_Meta (std::string _path, size_t beg, size_t end) {
      path = std::move(_path);
      begin_offset = beg;
      end_offset = end;
    }

    friend ibinstream& operator<<(ibinstream& m, const LR_Meta& meta){
        m << meta.path.length();
        m.raw_bytes(meta.path.c_str(), meta.path.length());
        m << meta.begin_offset << meta.end_offset;
        return m;
    }

    friend obinstream& operator>>(obinstream& m, LR_Meta& meta)
    {
        size_t len;
        m >> len;

        if (len) {
          meta.path.resize(len);
          char *c = (char*)meta.path.c_str();
          char * data = (char*)m.raw_bytes(meta.path.length());
          memcpy(c, data, len);
        }

        m >> meta.begin_offset >> meta.end_offset;

        return m;
    }
}LR_Meta;

typedef struct LR_PKG{
public:
    std::string buf;
    size_t begin_offset, end_offset;
    int buf_index;

    LR_PKG(){}

    LR_PKG (std::string data, size_t beg, size_t end, int index){
      buf = std::move(data);
      begin_offset = beg;
      end_offset = end;
      buf_index = index;
    }

    friend ibinstream& operator<<(ibinstream& m, const LR_PKG& pkg){
        m << pkg.buf.length();
        m.raw_bytes(pkg.buf.c_str(), pkg.buf.length());
        m << pkg.begin_offset << pkg.end_offset << pkg.buf_index;
        return m;
    }

    friend obinstream& operator>>(obinstream& m, LR_PKG& pkg)
    {
        size_t len;
        m >> len;

        if (len) {
            pkg.buf.resize(len);
            char * data = (char*)m.raw_bytes(len);
            char *c = (char*)pkg.buf.c_str();
            memcpy(c, data, len);
        }

        m >> pkg.begin_offset >> pkg.end_offset >> pkg.buf_index;
        return m;
    }

    friend bool operator<(const LR_PKG &a, const LR_PKG &b) {
        return a.buf_index < b.buf_index;
    }
}LR_PKG;


typedef struct SR_PKG{
public:
    int done;
    char * buf;
    size_t size;

    SR_PKG(){}

    SR_PKG (int _done, const char * _buf , size_t _size){
        done = _done;
        if(done){
            size = _size;
            buf = new char[_size];
            memcpy(buf,_buf,_size);
        }else{
            size = 0;
            buf = NULL;
        }
    }

    ~SR_PKG(){
        if(done)
            delete [] buf;
    }

    friend ibinstream& operator<<(ibinstream& m, const SR_PKG& pkg){
        m << pkg.done;
        if(pkg.done){
            m << pkg.size;
            m.raw_bytes(pkg.buf,pkg.size);
        }
        return m;
    }

    friend obinstream& operator>>(obinstream& m, SR_PKG& pkg)
    {
        m >> pkg.done;
        if(pkg.done){
            m >> pkg.size;
            pkg.buf = new char[pkg.size];
            char * data = (char*)m.raw_bytes(pkg.size);
            memcpy(pkg.buf, data, pkg.size);
        }else{
            pkg.size = 0;
            pkg.buf = NULL;
        }
        return m;
    }
}SR_PKG;

typedef struct ATTR_Meta{
public:
    char * path;
    int path_len;

    ATTR_Meta(){}

    ATTR_Meta (const char * _path){
        path_len = strlen(_path);
        path = new char[path_len+1];
        memcpy(path, _path, path_len+1);
    }

    ~ATTR_Meta(){
        delete [] path;
    }

    friend ibinstream& operator<<(ibinstream& m, const ATTR_Meta& meta){
        m << meta.path_len;
        m.raw_bytes(meta.path, meta.path_len+1);
        return m;
    }

    friend obinstream& operator>>(obinstream& m, ATTR_Meta& meta)
    {
        m >> meta.path_len;

        meta.path = new char[meta.path_len+1];
        char * data = (char*)m.raw_bytes(meta.path_len+1);
        memcpy(meta.path, data, meta.path_len+1);

        return m;
    }
}ATTR_Meta;

enum Command
{
    INIT, S_READ, L_READ, S_WRITE, L_WRITE, G_ATTR
};

#endif /* MSG_H_ */
