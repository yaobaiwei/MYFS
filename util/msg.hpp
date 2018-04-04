/*
 *  Authors: Hongzhi Chen, Yunjian Zhao
 */

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
