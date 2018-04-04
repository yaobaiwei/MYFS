#Authors: Hongzhi Chen, Yunjian Zhao


include_directories(${PROJECT_SOURCE_DIR})

add_subdirectory(iniparser) 

add_library(util-objs OBJECT
    global.cpp
    mailbox.cpp
   	log.c
    )

add_library(myfs-util STATIC $<TARGET_OBJECTS:util-objs> $<TARGET_OBJECTS:iniparser-objs>)
