/*
 *  Authors: Hongzhi Chen, Yunjian Zhao
 */

#ifndef MAILBOX_HPP_
#define MAILBOX_HPP_

#include <mpi.h>
#include <vector>

#include "util/serialization.hpp"
#include "util/msg.hpp"

using namespace std;

class MailBox{
public:

int all_sum(int my_copy);
int master_sum(int my_copy);
void send(void* buf, int size, int dst);
int recv(void* buf, int size, int src);
void send_ibinstream(ibinstream& m, int dst);
obinstream recv_obinstream(int src);

template <class T>
void send_data(const T& data, int dst)
{
    ibinstream m;
    m << data;
    send_ibinstream(m, dst);
}

template <class T>
T recv_data(int src)
{
    obinstream um = recv_obinstream(src);
    T data;
    um >> data;
    return data;
}

template <class T>
void masterScatter(vector<T>& to_send)
{
    int* sendcounts = new int[_num_workers];
    int recvcount;
    int* sendoffset = new int[_num_workers];

    ibinstream m;
    int size = 0;
    for (int i = 0; i < _num_workers; i++) {
        if (i == _my_rank) {
            sendcounts[i] = 0;
        } else {
            m << to_send[i];
            sendcounts[i] = m.size() - size;
            size = m.size();
        }
    }
    MPI_Scatter(sendcounts, 1, MPI_INT, &recvcount, 1, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);

    for (int i = 0; i < _num_workers; i++) {
        sendoffset[i] = (i == 0 ? 0 : sendoffset[i - 1] + sendcounts[i - 1]);
    }
    char* sendbuf = m.get_buf();
    char* recvbuf;

    MPI_Scatterv(sendbuf, sendcounts, sendoffset, MPI_CHAR, recvbuf, recvcount, MPI_CHAR, MASTER_RANK, MPI_COMM_WORLD);

    delete[] sendcounts;
    delete[] sendoffset;
}

template <class T>
void slaveScatter(T& to_get)
{
    int* sendcounts;
    int recvcount;
    int* sendoffset;

    MPI_Scatter(sendcounts, 1, MPI_INT, &recvcount, 1, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);

    char* sendbuf;
    char* recvbuf = new char[recvcount];

    MPI_Scatterv(sendbuf, sendcounts, sendoffset, MPI_CHAR, recvbuf, recvcount, MPI_CHAR, MASTER_RANK, MPI_COMM_WORLD);
    obinstream um(recvbuf, recvcount);
    um >> to_get;
}

template <class T>
void masterGather(vector<T>& to_get)
{
    int sendcount = 0;
    int* recvcounts = new int[_num_workers];
    int* recvoffset = new int[_num_workers];

    MPI_Gather(&sendcount, 1, MPI_INT, recvcounts, 1, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);

    for (int i = 0; i < _num_workers; i++) {
        recvoffset[i] = (i == 0 ? 0 : recvoffset[i - 1] + recvcounts[i - 1]);
    }
    char* sendbuf;
    int recv_tot = recvoffset[_num_workers - 1] + recvcounts[_num_workers - 1];
    char* recvbuf = new char[recv_tot];

    MPI_Gatherv(sendbuf, sendcount, MPI_CHAR, recvbuf, recvcounts, recvoffset, MPI_CHAR, MASTER_RANK, MPI_COMM_WORLD);

    obinstream um(recvbuf, recv_tot);
    for (int i = 0; i < _num_workers; i++) {
        if (i == _my_rank)
            continue;
        um >> to_get[i];
    }

    delete[] recvcounts;
    delete[] recvoffset;
}

template <class T>
void slaveGather(T& to_send)
{
    int sendcount;
    int* recvcounts;
    int* recvoffset;

    ibinstream m;
    m << to_send;
    sendcount = m.size();

    MPI_Gather(&sendcount, 1, MPI_INT, recvcounts, 1, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);

    char* sendbuf = m.get_buf();
    char* recvbuf;
    MPI_Gatherv(sendbuf, sendcount, MPI_CHAR, recvbuf, recvcounts, recvoffset, MPI_CHAR, MASTER_RANK, MPI_COMM_WORLD);
}

template <class T>
void masterBcast(T& to_send)
{
    ibinstream m;
    m << to_send;
    int size = m.size();
    MPI_Bcast(&size, 1, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);

    char* sendbuf = m.get_buf();
    MPI_Bcast(sendbuf, size, MPI_CHAR, MASTER_RANK, MPI_COMM_WORLD);
}

template <class T>
void slaveBcast(T& to_get)
{
    int size;
    MPI_Bcast(&size, 1, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);

    char* recvbuf = new char[size];
    MPI_Bcast(recvbuf, size, MPI_CHAR, MASTER_RANK, MPI_COMM_WORLD);

    obinstream um(recvbuf, size);
    um >> to_get;
}

void masterBcastCMD(Command cmd)
{
	int msg = cmd;
	MPI_Bcast(&msg, 1, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);
}

Command slaveBcastCMD()
{
	int msg;
	MPI_Bcast(&msg, 1, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);
	Command cmd = static_cast<Command>(msg);
	return cmd;
}

};

#endif
