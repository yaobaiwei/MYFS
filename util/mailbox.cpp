/*
 *  Authors: Hongzhi Chen, Yunjian Zhao
 */
#include <mpi.h>

#include "util/mailbox.hpp"
#include "util/serialization.hpp"
#include "util/msg.hpp"

int MailBox::all_sum(int my_copy)
{
    int tmp;
    MPI_Allreduce(&my_copy, &tmp, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
    return tmp;
}

int MailBox::master_sum(int my_copy)
{
    int tmp = 0;
    MPI_Reduce(&my_copy, &tmp, 1, MPI_INT, MPI_SUM, MASTER_RANK, MPI_COMM_WORLD);
    return tmp;
}

void MailBox::send(void* buf, int size, int dst)
{
    MPI_Send(buf, size, MPI_CHAR, dst, COMMUN_CHANNEL, MPI_COMM_WORLD);
}

int MailBox::recv(void* buf, int size, int src)
{
	MPI_Status status;
    MPI_Recv(buf, size, MPI_CHAR, src, COMMUN_CHANNEL, MPI_COMM_WORLD, &status);
    return status.MPI_SOURCE;
}

void MailBox::send_ibinstream(ibinstream& m, int dst)
{
	size_t size = m.size();
	send(&size, sizeof(size_t), dst);
    send(m.get_buf(), m.size(), dst);
}

obinstream MailBox::recv_obinstream(int src)
{
	size_t size;
	src = recv(&size, sizeof(size_t), src); //must receive the content (paired with the msg-size) from the msg-size source
    char* buf = new char[size];
    recv(buf, size, src);
    return obinstream(buf, size);
}
