/*
 *  Authors: Hongzhi Chen, Yunjian Zhao
 */

#include "util/global.hpp"
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <iostream>

using namespace std;

int _my_rank;
int _num_workers;
bool _force_write;

void init_worker(int * argc, char*** argv)
{
	int provided;
	MPI_Init_thread(argc, argv, MPI_THREAD_MULTIPLE, &provided);
	if(provided != MPI_THREAD_MULTIPLE)
	{
		printf("MPI do not Support Multiple thread\n");
		exit(0);
	}
	MPI_Comm_size(MPI_COMM_WORLD, &_num_workers);
	MPI_Comm_rank(MPI_COMM_WORLD, &_my_rank);
}

void worker_finalize()
{
	MPI_Finalize();
}

void worker_barrier()
{
	MPI_Barrier(MPI_COMM_WORLD);
}

void load_config(Params & params)
{
	dictionary *ini;
	int val, val_not_found = -1;
	char *str;
	char *str_not_found = "null";

	const char* MYFS_HOME = getenv("MYFS_HOME");
	if(MYFS_HOME == NULL)
	{
		fprintf(stderr, "PLease Set the Environment Variable: MYFS_HOME.\nExits.\n");
		exit(-1);
	}
	std::string conf_path(MYFS_HOME);
	conf_path.append("/MYFS_Config.ini");
	ini = iniparser_load(conf_path.c_str());
	if(ini == NULL)
	{
		fprintf(stderr, "Can not open %s.\nExits.\n", "MYFS_Config.ini");
		exit(-1);
	}

	// [CONFIG]
	str = iniparser_getstring(ini,"CONFIG:DATANODE_ROOT", str_not_found);
	if(strcmp(str, str_not_found)!=0) params.datanode_root = str;
	else
	{
		fprintf(stderr, "Must config the datanode root path.\nExits.\n");
		exit(-1);
	}

	str = iniparser_getstring(ini,"CONFIG:CLIENT_ROOT", str_not_found);
	if(strcmp(str, str_not_found)!=0) params.client_root = str;
	else
	{
		fprintf(stderr, "Must config the client root path.\nExits.\n");
		exit(-1);
	}

	val = iniparser_getint(ini, "CONFIG:THETA", val_not_found);
	if(val!=val_not_found) params.theta=val;
	else
	{
		fprintf(stderr, "Must config the theta for MYFS.\nExits.\n");
		exit(-1);
	}

	val = iniparser_getint(ini, "CONFIG:FORCE_WRITE", val_not_found);
	if(val!=val_not_found) _force_write = val ? true : false;
	else
	{
		fprintf(stderr, "Must config the force_write for MYFS.\nExits.\n");
		exit(-1);
	}

	iniparser_freedict(ini);
}

void mk_dir(const char *dir)
{
	char tmp[256];
	char *p = NULL;
	size_t len;

	snprintf(tmp, sizeof(tmp), "%s", dir);
	len = strlen(tmp);
	if(tmp[len - 1] == '/') tmp[len - 1] = '\0';
	for (p = tmp + 1; *p; p++)
	{
		if (*p == '/')
		{
			*p = 0;
			mkdir(tmp, S_IRWXU);
			*p = '/';
		}
	}
	mkdir(tmp, S_IRWXU);
}

int _nid = 1;
int get_next_nid(){
	_nid = _nid % (get_num_workers()-1) + 1;
	return _nid;
}

void rm_dir(string path)
{
	DIR* dir = opendir(path.c_str());
	struct dirent * file;
	while ((file = readdir(dir)) != NULL)
	{
		if(strcmp(file->d_name, ".") == 0 || strcmp(file->d_name, "..") == 0)
			continue;
		string filename = path + "/" + file->d_name;
		remove(filename.c_str());
	}
	if (rmdir(path.c_str()) == -1)
	{
		perror ("The following error occurred");
		exit(-1);
	}
}

void check_dir(string path)
{
	if(access(path.c_str(), F_OK) == 0 )
	{
		if (_force_write)
		{
			rm_dir(path);
			mk_dir(path.c_str());
		}
	}
	else
	{
		mk_dir(path.c_str());
	}
}
