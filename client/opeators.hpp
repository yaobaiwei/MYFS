/*
 *  Authors: Hongzhi Chen, Yunjian Zhao
 */

#ifndef OPEATORS_H_
#define OPEATORS_H_

extern "C" {
#include "util/config.h"
#include "util/params.h"
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
#include <unistd.h>
#include <sys/types.h>
#include <utime.h>

void myfs_fullpath(char fpath[PATH_MAX], const char *path);
int myfs_getattr(const char *path, struct stat *statbuf);
int myfs_readlink(const char *path, char *link, size_t size);
int myfs_mknod(const char *path, mode_t mode, dev_t dev);
int myfs_mkdir(const char *path, mode_t mode);
int myfs_unlink(const char *path);
int myfs_rmdir(const char *path);
int myfs_symlink(const char *path, const char *link);
int myfs_rename(const char *path, const char *newpath);
int myfs_link(const char *path, const char *newpath);
int myfs_chmod(const char *path, mode_t mode);
int myfs_chown(const char *path, uid_t uid, gid_t gid);
int myfs_truncate(const char *path, off_t newsize);
int myfs_utime(const char *path, struct utimbuf *ubuf);
int myfs_open(const char *path, struct fuse_file_info *fi);
int myfs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi);
int myfs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi);
int myfs_statfs(const char *path, struct statvfs *statv);
int myfs_release(const char *path, struct fuse_file_info *fi);
int myfs_opendir(const char *path, struct fuse_file_info *fi);
int myfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi);
int myfs_releasedir(const char *path, struct fuse_file_info *fi);
void *myfs_init(struct fuse_conn_info *conn);
void myfs_destroy(void *userdata);
int myfs_access(const char *path, int mask);



class Opeators{
public:
	int mount(int argc, char** argv, struct myfs_state * myfs_data);

private:
	const struct fuse_operations _myfs_oper = {
		/*getattr :*/ myfs_getattr,
		/*readlink :*/ myfs_readlink,
		/*getdir :*/ NULL,
		/*mknod :*/ myfs_mknod,
		/*mkdir :*/ myfs_mkdir,
		/*unlink :*/ myfs_unlink,
		/*rmdir :*/ myfs_rmdir,
		/*symlink :*/ myfs_symlink,
		/*rename :*/ myfs_rename,
		/*link :*/ myfs_link,
		/*chmod :*/ myfs_chmod,
		/*chown :*/ myfs_chown,
		/*truncate :*/ myfs_truncate,
		/*utime :*/ myfs_utime,
		/*open :*/ myfs_open,
		/*read :*/ myfs_read,
		/*write :*/ myfs_write,
		/*statfs :*/ myfs_statfs,
		/*flush :*/ NULL,
		/*release :*/ myfs_release,
		/*fsync :*/ NULL,
		/*setxattr :*/ NULL,
		/*getxattr :*/ NULL,
		/*listxattr :*/ NULL,
		/*removexattr :*/ NULL,
		/*opendir :*/ myfs_opendir,
		/*readdir :*/ myfs_readdir,
		/*releasedir :*/ myfs_releasedir,
		/*fsyncdir :*/ NULL,
		/*init :*/ myfs_init,
		/*destroy :*/ myfs_destroy,
		/*access :*/ myfs_access,
		/*create :*/ NULL,
		/*ftruncate :*/ NULL,
		/*fgetattr :*/ NULL,
		/*lock :*/ NULL,
		/*utimens :*/ NULL,
		/*bmap :*/ NULL,
		/*flag_nullpath_ok :*/ 1,
		/*flag_nopath :*/ 1,
		/*flag_utime_omit_ok :*/ 1,
		/*flag_reserved :*/ 29,
		/*ioctl :*/ NULL,
		/*poll :*/ NULL,
		/*write_buf :*/ NULL,
		/*read_buf :*/ NULL,
		/*flock :*/ NULL,
		/*fallocate :*/ NULL,
	};
};

#endif /* OPEATORS_H_ */
