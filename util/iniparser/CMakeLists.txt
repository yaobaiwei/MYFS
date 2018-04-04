#Authors: Hongzhi Chen, Yunjian Zhao


include_directories(${PROJECT_SOURCE_DIR})

add_library(iniparser-objs OBJECT
    dictionary.c
    iniparser.c
    )