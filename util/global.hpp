/*
 *  Authors: Hongzhi Chen, Yunjian Zhao
 */

#ifndef GLOBAL_HPP_
#define GLOBAL_HPP_


#include <mpi.h>

#include <unistd.h>
#include <stdlib.h>
#include <string>
#include <ext/hash_set>
#include <ext/hash_map>

extern "C" {
#include "util/iniparser/iniparser.h"
}

//============================
#define hash_map __gnu_cxx::hash_map
#define hash_set __gnu_cxx::hash_set

#define MASTER_RANK 0
#define COMMUN_CHANNEL 100

extern int _my_rank;
extern int _num_workers;
extern int _nid;
extern bool _force_write;

inline int get_worker_id()
{
	return _my_rank;
}

inline int get_num_workers()
{
	return _num_workers;
}

void init_worker(int* argc, char*** argv);
void worker_finalize();
void worker_barrier();

struct Params
{
	std::string datanode_root;
	std::string client_root;
	int theta;
};

void load_config(Params & params);

void mk_dir(const char *dir);
void rm_dir(std::string path);
void check_dir(std::string path);
int get_next_nid();

#endif /* GLOBAL_HPP_ */
