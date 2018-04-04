#Authors: Hongzhi Chen, Yunjian Zhao

include_directories(${PROJECT_SOURCE_DIR})
#link_directories(${FUSE_LIBRARIES})

add_library(client-objs OBJECT
	client.cpp
	opeators.cpp
    )

add_library(myfs-client STATIC $<TARGET_OBJECTS:client-objs>)
