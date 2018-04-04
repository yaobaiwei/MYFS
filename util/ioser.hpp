#ifndef IOSER_H
#define IOSER_H

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <vector>
#include <list>
#include <set>
#include <string>
#include <map>

using namespace std;

#define STREAM_MEMBUF_SIZE 65536 //64k

//-------------------------------------

class ifbinstream {

private:
    char* membuf;
    int bufpos;
    size_t totpos;
    FILE * file;

public:

    ifbinstream()//empty
	{
		file = NULL;
		bufpos = 0;
		totpos = 0;
		membuf = new char[STREAM_MEMBUF_SIZE];
	}

    ifbinstream(const char* path)
    {
    	file = fopen(path, "wb");
    	bufpos = 0;
    	totpos = 0;
    	membuf = new char[STREAM_MEMBUF_SIZE];
    }

    inline void bufflush()
    {
    	fwrite(membuf, 1, bufpos, file);
    }

    ~ifbinstream()
	{
    	delete membuf;
    	if(file == NULL) return; //already closed
    	if(bufpos > 0) bufflush();
    	fclose(file);
	}

    inline size_t size()
    {
        return totpos;
    }

    void raw_byte(char c)
    {
    	if(bufpos == STREAM_MEMBUF_SIZE)
    	{
    		bufflush();
    		bufpos = 0;
    	}
    	membuf[bufpos] = c;
    	bufpos++;
        totpos++;
    }

    void raw_bytes(const void* ptr, int size)
    {
    	totpos += size;
    	int gap = STREAM_MEMBUF_SIZE - bufpos;
    	char * cptr = (char *)ptr;
    	if(gap < size)
    	{
    		memcpy(membuf + bufpos, cptr, gap);
    		bufpos = STREAM_MEMBUF_SIZE; //useful for correct exec of bufflush()
    		bufflush();
    		size -= gap;
    		cptr += gap;
    		while(size > STREAM_MEMBUF_SIZE)
    		{
    			memcpy(membuf, cptr, STREAM_MEMBUF_SIZE);
    			bufflush();
    			size -= STREAM_MEMBUF_SIZE;
    			cptr += STREAM_MEMBUF_SIZE;
    		}
    		memcpy(membuf, cptr, size);
    		bufpos = size;
    	}
    	else
    	{
    		memcpy(membuf + bufpos, ptr, size);
    		bufpos += size;
    	}
    }

    void close() //also for flushing
    {
    	if(file == NULL) return; //already closed
    	if(bufpos > 0) bufflush();
    	fclose(file);
    	file = NULL; //set status to closed
    }

    void open(const char* path) //it does not check whether you closed previous file
    {
    	file = fopen(path,"wb");
		bufpos = 0;
		totpos = 0;
    }

    bool is_open()
    {
    	return file != NULL;
    }

};

//-------------------------------------

class ofbinstream {

private:
	char* membuf;
	int bufpos;
	int bufsize; //membuf may not be full (e.g. last batch)
	size_t totpos;
	size_t filesize;
	FILE * file;

public:
	inline void fill()
	{
		bufsize = fread(membuf, 1, STREAM_MEMBUF_SIZE, file);
		bufpos = 0;
	}

	ofbinstream()
	{
		membuf = new char[STREAM_MEMBUF_SIZE];
		file = NULL; //set status to closed
	}

	ofbinstream(const char* path)
	{
		membuf = new char[STREAM_MEMBUF_SIZE];
		file = fopen(path, "rb");
		//get file size
		filesize = -1;
		struct stat statbuff;
		if(stat(path, &statbuff) == 0) filesize = statbuff.st_size;
		//get first batch
		fill();
		totpos = 0;
	}

	bool open(const char* path) //return whether the file exists
	{
		file = fopen(path, "rb");
		if(file == NULL) return false;
		//get file size
		filesize = -1;
		struct stat statbuff;
		if(stat(path, &statbuff) == 0) filesize = statbuff.st_size;
		//get first batch
		fill();
		totpos = 0;
		return true;
	}

	inline size_t size()
	{
		return filesize;
	}

	inline bool eof()
	{
		return totpos >= filesize;
	}

    ~ofbinstream()
    {
    	delete membuf;
    	if(file == NULL) return; //already closed
		fclose(file);
    }

    char raw_byte()
    {
    	totpos++;
    	if(bufpos == bufsize) fill();
        return membuf[bufpos++];
    }

    void* raw_bytes(unsigned int n_bytes)
    {
    	totpos += n_bytes;
    	int gap = bufsize - bufpos;
    	if(gap >= n_bytes)
    	{
    		char* ret = membuf + bufpos;
    		bufpos += n_bytes;
			return ret;
    	}
    	else
    	{
    		//copy the last gap-batch to head of membuf
    		//!!! require that STREAM_MEMBUF_SIZE >= n_bytes !!!
    		memcpy(membuf, membuf + bufpos, gap);
    		//gap-shifted refill
    		bufsize = gap + fread(membuf + gap, 1, STREAM_MEMBUF_SIZE - gap, file);
    		bufpos = n_bytes;
    		return membuf;
    	}
    }

    void close()
	{
    	if(file == NULL) return; //already closed
		fclose(file);
		file = NULL; //set status to closed
	}

    //=============== add skip function ===============
    void skip(int num_bytes)
    {
    	totpos += num_bytes;
    	if(totpos >= filesize) return; //eof
    	bufpos += num_bytes; //done if bufpos < bufsize
    	if(bufpos >= bufsize)
    	{
    		fseek(file, bufpos - bufsize, SEEK_CUR);
    		fill();
    	}
    }
};

#endif
