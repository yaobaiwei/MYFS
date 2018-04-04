/*
 *  Authors: Hongzhi Chen, Yunjian Zhao
 */

#ifndef CLIENT_HPP_
#define CLIENT_HPP_

extern "C" {
#include "util/config.h"
#include "util/params.h"
}

#include <iostream>
#include <string>

using namespace std;

class Client{
	public:
		Client (int argc, char** argv, string root, int theta){
			_argc = argc;
			_argv = argv;
			_root = root;
			_theta = theta;
		}

		int mount();
		void myfs_usage();

	private:
		int _argc;
		char ** _argv;
		string _root;
		int _theta; //for data splitting
};

#endif /* CLIENT_HPP_ */
