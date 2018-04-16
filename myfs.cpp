/*
 *  Authors: Hongzhi Chen, Yunjian Zhao
 */

#include "util/global.hpp"
#include "client/client.hpp"
#include "datanode/datanode.hpp"

int main(int argc, char* argv[])
{
    //argv
    //mpiexec -n [process number] -f node.config ./run [argv]
    init_worker(&argc, &argv);

    Params params;
    load_config(params);

    if(get_worker_id() == MASTER_RANK){
        Client client(argc, argv, params.client_root, params.theta);
        client.mount();
    }else{
        Datanode datanode(params.datanode_root, get_worker_id());
        datanode.run();
    }

    worker_finalize();
    return 0;
}
