# This is a README

This is a joint work with CHEN Hongzhi.

In case of any problem, welcome to the my [repo](https://github.com/zzxx-husky/Lala_Land) on github and raise any issue.

There are sample IOZone reports generated using my own computer. The reports are also inside this zip file. One is a text file with named **report.txt** and the other one is an excel compatible binary file named **output.wks**.

1. Make sure you have mpi installed in your machines. <br>
   a. Change the C and C++ compiler (CMAKE_C_COMPILER and CMAKE_CXX_COMPILER) in the `./CMakeLists.txt`. <br>
   b. For 32 bit machines, change `-m64` to `-m32` in both CMAKE_CXX_FLAGS and CMAKE_C_FLAGS in the `./CMakeLists.txt`. <br>
2. Build <br>
   a. Add an environment variable `FUSE_ROOT` to specify the root directory of fuse.<br>
   b. Add an environment variable `MYFS_HOME` to specify the directory of this project.<br>
   c. Build the project: <br>
    ```
    mkdir build
    cd build
    cmake .. -DCMAKE_BUILD_TYPE=Relase
    make all -j4
    ```
   d. The executable `myfs` is generated under `../release` folder.
3. Change configuration in `MYFS_Config.ini`<br>
   a. `DATANODE_ROOT` An available absolute directory on datanode to store the data.<br>
   b. `CLIENT_ROOT` An auxilary directory on the machine where we mount fuse.<br>
   c. `THETA` A threshold in bytes to distinguish large file and small file.<br>
4. Mount (in case of fire)<br>
    ```
    cd ../release
    # single machine multi processes
    mpiexec -n 3 ./myfs -s -f [mount_dir]
    # multi machines multi processes
    mpiexec -n 3 -host [hostname_file] ./myfs -s -f [mount_dir]
    ```
5. Run iozone:
    ```
    ./iozone -Rab output.wks -g 2G -i 0 -i 1 -f [mount_dir]/temp
    ```
6. Unmount <br>
    ```
    fusermount -u [mount_dir]
    # then remove the `DATANODE_ROOT` on the datanodes.
    ```
7. Notes:<br>
   a. For `Unmount`, make sure you remove the `DATANODE_ROOT` on the datanodes!! Otherwise, you cannot mount again.<br>
   b. Before `Mount`, make sure you create the `[mount_dir]` on the machine where we mount fuse!!<br>
   c. For multi mahcines, the first machine is where we mount fuse. Usually we should list local machine as the first machine.<br>
