#Authors: Hongzhi Chen, Yunjian Zhao

include_directories(${PROJECT_SOURCE_DIR})
#link_directories(${FUSE_LIBRARIES})

add_library(datanode-objs OBJECT
	datanode.cpp
    )

add_library(myfs-datanode STATIC $<TARGET_OBJECTS:datanode-objs>)
