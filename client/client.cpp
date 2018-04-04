/*
 *  Authors: Hongzhi Chen, Yunjian Zhao
 */

extern "C"{
#include "util/log.c"
}
#include "client/client.hpp"
#include "client/opeators.hpp"
#include "util/global.hpp"

#include <unistd.h>
#include <stdlib.h>
#include<iostream>

void Client::myfs_usage()
{
    fprintf(stderr, "usage:  myfs [FUSE and mount options] mount_path\n");
    abort();
}

int Client::mount(){
	int fuse_stat;
	struct myfs_state *myfs_data;

	if ((getuid() == 0) || (geteuid() == 0)) {
		fprintf(stderr, "Running MYFS as root is not suggested\n");
		return 1;
	}

	if ((_argc < 2) || (_argv[_argc-1][0] == '-'))
		myfs_usage();

	myfs_data = (struct myfs_state *)malloc(sizeof(struct myfs_state));

	myfs_data->rootdir = &(_root[0u]);
	myfs_data->logfile = log_open();

	check_dir(_root);

	// turn over control to fuse
	fprintf(stderr, "about to call fuse_main\n");
	Opeators opers;
	fuse_stat = opers.mount(_argc, _argv, myfs_data);
	fprintf(stderr, "fuse_main returned %d\n", fuse_stat);

	return fuse_stat;
}
