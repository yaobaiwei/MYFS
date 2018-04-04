# Find the ZMQ includes and library
#
#  ZMQ_INCLUDE_DIR - where to find zmq.h, etc.
#  ZMQ_LIBRARIES   - List of libraries when using ZMQ.
#  ZMQ_FOUND       - True if ZMQ lib is found.

# check if already in cache, be silent
IF (ZMQ_INCLUDE_DIR)
    SET (ZMQ_FIND_QUIETLY TRUE)
ENDIF (ZMQ_INCLUDE_DIR)

# find includes
FIND_PATH (ZMQ_INCLUDE_DIR zmq.h
        /usr/include
        /usr/local/include
	/usr/local/libzmq/include
        )

# find lib
SET(ZMQ_NAMES zmq)

FIND_LIBRARY(ZMQ_LIBRARIES
        NAMES ${ZMQ_NAMES}
        PATHS /lib64 /lib /usr/lib64 /usr/lib /usr/local/lib64 /usr/local/lib /usr/local/libzmq/lib
        )

include ("FindPackageHandleStandardArgs")
find_package_handle_standard_args ("ZMQ" DEFAULT_MSG
        ZMQ_INCLUDE_DIR ZMQ_LIBRARIES)

mark_as_advanced (ZMQ_INCLUDE_DIR ZMQ_LIBRARIES)
