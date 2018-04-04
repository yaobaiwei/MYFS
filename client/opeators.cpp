/*
 *  Authors: Hongzhi Chen, Yunjian Zhao
 */

extern "C" {
#include "util/config.h"
#include "util/params.h"
#include "util/log.h"
}

#include <fuse.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <limits.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <utime.h>
#include <iostream>

#ifdef HAVE_SYS_XATTR_H
#include <sys/xattr.h>
#endif

#include <mpi.h>
#include "client/opeators.hpp"
#include "util/mailbox.hpp"
#include "util/serialization.hpp"

char open_fname[PATH_MAX];

void set_fname(const char * path){
    memcpy(open_fname, path, strlen(path));
    open_fname[strlen(path)] = '\0';
}

const char * get_fname(){
	return open_fname;
}

void myfs_fullpath(char fpath[PATH_MAX], const char *path)
{
    strcpy(fpath, MYFS_DATA->rootdir);
    strncat(fpath, path, strlen(path));
    log_msg("    myfs_fullpath:  rootdir = \"%s\", path = \"%s\", fpath = \"%s\"\n", MYFS_DATA->rootdir, path, fpath);
}


int myfs_getattr(const char *path, struct stat *statbuf)
{
    int retstat;
    char fpath[PATH_MAX];

    log_msg("\nmyfs_getattr(path=\"%s\", statbuf=0x%08x)\n", path, statbuf);
    myfs_fullpath(fpath, path);
    retstat = log_syscall("lstat", lstat(fpath, statbuf), 0);

    //----
    MailBox mb;
    mb.masterBcastCMD(G_ATTR);
    int wid;
	mb.recv(&wid,sizeof(wid), MPI_ANY_SOURCE);

	ATTR_Meta meta(path);
	mb.send_data(meta, wid);

    int size;
	mb.recv(&size,sizeof(int), wid);
	statbuf->st_size = (off_t)size;
	//----

    log_stat(statbuf);

    return retstat;
}

int myfs_readlink(const char *path, char *link, size_t size)
{
    int retstat;
    char fpath[PATH_MAX];

    log_msg("\nmyfs_readlink(path=\"%s\", link=\"%s\", size=%d)\n", path, link, size);
    myfs_fullpath(fpath, path);

    retstat = log_syscall("readlink", readlink(fpath, link, size - 1), 0);
    if (retstat >= 0) {
    	link[retstat] = '\0';
    	retstat = 0;
    	log_msg("    link=\"%s\"\n", link);
    }

    return retstat;
}


int myfs_mknod(const char *path, mode_t mode, dev_t dev)
{
    int retstat;
    char fpath[PATH_MAX];

    log_msg("\nmyfs_mknod(path=\"%s\", mode=0%3o, dev=%lld)\n", path, mode, dev);
    myfs_fullpath(fpath, path);

    if (S_ISREG(mode)) {
	retstat = log_syscall("open", open(fpath, O_CREAT | O_EXCL | O_WRONLY, mode), 0);
	if (retstat >= 0)
	    retstat = log_syscall("close", close(retstat), 0);
    } else
	if (S_ISFIFO(mode))
	    retstat = log_syscall("mkfifo", mkfifo(fpath, mode), 0);
	else
	    retstat = log_syscall("mknod", mknod(fpath, mode, dev), 0);

    return retstat;
}


int myfs_mkdir(const char *path, mode_t mode)
{
    char fpath[PATH_MAX];

    log_msg("\nmyfs_mkdir(path=\"%s\", mode=0%3o)\n", path, mode);
    myfs_fullpath(fpath, path);

    return log_syscall("mkdir", mkdir(fpath, mode), 0);
}


//TODO
/** Remove a file */
int myfs_unlink(const char *path)
{
    char fpath[PATH_MAX];

    log_msg("myfs_unlink(path=\"%s\")\n", path);
    myfs_fullpath(fpath, path);

    return log_syscall("unlink", unlink(fpath), 0);
}


//TODO
/** Remove a directory */
int myfs_rmdir(const char *path)
{
    char fpath[PATH_MAX];

    log_msg("myfs_rmdir(path=\"%s\")\n", path);
    myfs_fullpath(fpath, path);

    return log_syscall("rmdir", rmdir(fpath), 0);
}

int myfs_symlink(const char *path, const char *link)
{
    char flink[PATH_MAX];

    log_msg("\nmyfs_symlink(path=\"%s\", link=\"%s\")\n", path, link);
    myfs_fullpath(flink, link);

    return log_syscall("symlink", symlink(path, flink), 0);
}


int myfs_rename(const char *path, const char *newpath)
{
    char fpath[PATH_MAX];
    char fnewpath[PATH_MAX];

    log_msg("\nmyfs_rename(fpath=\"%s\", newpath=\"%s\")\n", path, newpath);
    myfs_fullpath(fpath, path);
    myfs_fullpath(fnewpath, newpath);

    return log_syscall("rename", rename(fpath, fnewpath), 0);
}

int myfs_link(const char *path, const char *newpath)
{
    char fpath[PATH_MAX], fnewpath[PATH_MAX];

    log_msg("\nmyfs_link(path=\"%s\", newpath=\"%s\")\n", path, newpath);
    myfs_fullpath(fpath, path);
    myfs_fullpath(fnewpath, newpath);

    return log_syscall("link", link(fpath, fnewpath), 0);
}

int myfs_chmod(const char *path, mode_t mode)
{
    char fpath[PATH_MAX];

    log_msg("\nmyfs_chmod(fpath=\"%s\", mode=0%03o)\n", path, mode);
    myfs_fullpath(fpath, path);

    return log_syscall("chmod", chmod(fpath, mode), 0);
}

int myfs_chown(const char *path, uid_t uid, gid_t gid)
{
    char fpath[PATH_MAX];

    log_msg("\nmyfs_chown(path=\"%s\", uid=%d, gid=%d)\n", path, uid, gid);
    myfs_fullpath(fpath, path);

    return log_syscall("chown", chown(fpath, uid, gid), 0);
}


int myfs_truncate(const char *path, off_t newsize)
{
    char fpath[PATH_MAX];

    log_msg("\nmyfs_truncate(path=\"%s\", newsize=%lld)\n", path, newsize);
    myfs_fullpath(fpath, path);

    return log_syscall("truncate", truncate(fpath, newsize), 0);
}


int myfs_utime(const char *path, struct utimbuf *ubuf)
{
    char fpath[PATH_MAX];

    log_msg("\nmyfs_utime(path=\"%s\", ubuf=0x%08x)\n", path, ubuf);
    myfs_fullpath(fpath, path);

    return log_syscall("utime", utime(fpath, ubuf), 0);
}


int myfs_open(const char *path, struct fuse_file_info *fi)
{
    int retstat = 0;
    char fpath[PATH_MAX];

    set_fname(path);
    log_msg("\nmyfs_open(path\"%s\", fi=0x%08x)\n", path, fi);
    myfs_fullpath(fpath, path);

    int fd = log_syscall("open", open(fpath, fi->flags), 0);
    if (fd < 0)
    	retstat = log_error("open");

    fi->fh = fd;
    log_fi(fi);

    return retstat;
}


//TODO
int myfs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    log_msg("\nmyfs_read(path=\"%s\", buf=0x%08x, size=%d, offset=%lld, fi=0x%08x)\n", get_fname(), buf, size, offset, fi);
    log_fi(fi);

    MailBox mb;
    mb.masterBcastCMD(S_READ);

    int wid;
    mb.recv(&wid,sizeof(wid),MPI_ANY_SOURCE);

    SR_Meta meta(get_fname(), size, offset);
    mb.send_data(meta, wid);

    SR_PKG pkg = mb.recv_data<SR_PKG>(wid);

    if(pkg.done){
    	//read successfully
    	memcpy(buf, pkg.buf, pkg.size);
    }
    return pkg.size;
}

//TODO
int myfs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    int retstat = 0;

    log_msg("\nmyfs_write(path=\"%s\", buf=0x%08x, size=%d, offset=%lld, fi=0x%08x)\n", get_fname(), buf, size, offset, fi );
    log_fi(fi);

    MailBox mb;
    mb.masterBcastCMD(S_WRITE);

    SW_Meta pkg(get_fname(), buf, size, offset);
    mb.masterBcast(pkg);

    int len;
    mb.recv(&len,sizeof(len),MPI_ANY_SOURCE);

    return len;
}


int myfs_statfs(const char *path, struct statvfs *statv)
{
    int retstat = 0;
    char fpath[PATH_MAX];

    log_msg("\nmyfs_statfs(path=\"%s\", statv=0x%08x)\n", path, statv);
    myfs_fullpath(fpath, path);

    retstat = log_syscall("statvfs", statvfs(fpath, statv), 0);

    log_statvfs(statv);

    return retstat;
}


int myfs_release(const char *path, struct fuse_file_info *fi)
{
    log_msg("\nmyfs_release(path=\"%s\", fi=0x%08x)\n", path, fi);
    log_fi(fi);

    return log_syscall("close", close(fi->fh), 0);
}


int myfs_opendir(const char *path, struct fuse_file_info *fi)
{
    DIR *dp;
    int retstat = 0;
    char fpath[PATH_MAX];

    log_msg("\nmyfs_opendir(path=\"%s\", fi=0x%08x)\n", path, fi);
    myfs_fullpath(fpath, path);

    dp = opendir(fpath);
    log_msg("    opendir returned 0x%p\n", dp);
    if (dp == NULL)
	retstat = log_error("myfs_opendir opendir");

    fi->fh = (intptr_t) dp;

    log_fi(fi);

    return retstat;
}

int myfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
    int retstat = 0;
    DIR *dp;
    struct dirent *de;

    log_msg("\nmyfs_readdir(path=\"%s\", buf=0x%08x, filler=0x%08x, offset=%lld, fi=0x%08x)\n", path, buf, filler, offset, fi);
    // once again, no need for fullpath -- but note that I need to cast fi->fh
    dp = (DIR *) (uintptr_t) fi->fh;

    de = readdir(dp);
    log_msg("    readdir returned 0x%p\n", de);
    if (de == 0) {
    	retstat = log_error("myfs_readdir readdir");
    	return retstat;
    }

    do {
    	log_msg("calling filler with name %s\n", de->d_name);
    	if (filler(buf, de->d_name, NULL, 0) != 0)
    	{
    		log_msg("    ERROR myfs_readdir filler:  buffer full");
    		return -ENOMEM;
    	}
    } while ((de = readdir(dp)) != NULL);

    log_fi(fi);

    return retstat;
}


int myfs_releasedir(const char *path, struct fuse_file_info *fi)
{
    int retstat = 0;

    log_msg("\nmyfs_releasedir(path=\"%s\", fi=0x%08x)\n", path, fi);
    log_fi(fi);

    closedir((DIR *) (uintptr_t) fi->fh);

    return retstat;
}

//TODO
void * myfs_init(struct fuse_conn_info *conn)
{
    log_msg("\nmyfs_init()\n");

    log_conn(conn);
    log_fuse_context(fuse_get_context());

    MailBox mb;
    mb.masterBcastCMD(INIT);

    worker_barrier();
    return MYFS_DATA;
}

//TODO
void myfs_destroy(void *userdata)
{
    log_msg("\nmyfs_destroy(userdata=0x%08x)\n", userdata);
}


int myfs_access(const char *path, int mask)
{
    int retstat = 0;
    char fpath[PATH_MAX];

    log_msg("\nmyfs_access(path=\"%s\", mask=0%o)\n", path, mask);
    myfs_fullpath(fpath, path);

    retstat = access(fpath, mask);

    if (retstat < 0)
	retstat = log_error("myfs_access access");

    return retstat;
}

int Opeators::mount(int argc, char** argv, struct myfs_state * myfs_data){
	return fuse_main(argc, argv, &_myfs_oper, myfs_data);
}
