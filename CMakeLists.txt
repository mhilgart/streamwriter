cmake_minimum_required(VERSION 3.6)
project(streamwriter)
set(USE_SYSTEM_LIBRARIES TRUE)

set(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg -g")
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg -g")

# Package locations
find_package(Threads)

# includes
include_directories(.)
include_directories(modules/boost)
include_directories(modules/bitshuffle/src)
include_directories(modules/jsoncpp/dist/json)
include_directories(modules/bitshuffle/lz4)
include_directories(modules/onion/src modules/onion/src/bindings/cpp)
include_directories(modules/cppzmq)
if (USE_SYSTEM_LIBRARIES)
   include_directories(/usr/include/cbf)
   include_directories(/usr/include)
else()
   include_directories(modules/cbflib/CBFlib/include)
   include_directories(modules/libzmq/include)
endif()

# library paths
link_directories(modules/boost/stage/lib)
link_directories(modules/bitshuffle)
link_directories(modules/onion/build/src/onion modules/onion/build/src/bindings/cpp)
if (USE_SYSTEM_LIBRARIES)
   link_directories(/usr/lib64)
else()
   link_directories(modules/cbflib/CBFlib/solib)
   link_directories(modules/libzmq/.libs)
endif()

# source files
set(SOURCE_FILES src/writers/CBFWriter.cpp src/writers/CBFWriter.h src/reader/ZMQReader.cpp src/reader/ZMQReader.h src/reader/SeriesHeader.cpp src/reader/SeriesHeader.h src/reader/Image.cpp src/reader/Image.h src/common/Util.cpp src/common/Util.h src/reader/ZMQMultipartMessage.cpp src/reader/ZMQMultipartMessage.h src/http/Counter.cpp src/http/Counter.h modules/jsoncpp/dist/jsoncpp.cpp src/http/PathConfig.cpp src/http/PathConfig.h src/main/StreamWriter.h src/common/HttpGet.cpp src/common/HttpGet.h src/common/Log.h src/common/Log.cpp src/common/NetStat.cpp src/common/NetStat.h src/http/Statistics.cpp src/http/Statistics.h src/http/HTTPServer.cpp src/http/HTTPServer.h src/main/Config.cpp src/main/Config.h)

# output executable
add_executable(streamwriter src/main/StreamWriter.cpp ${SOURCE_FILES})

# libbitshuffle
set(LIBBITSHUFFLE_SO ${CMAKE_SOURCE_DIR}/modules/bitshuffle/libbitshuffle.a)
add_custom_command(
	OUTPUT ${LIBBITSHUFFLE_SO}
	COMMAND g++ -shared -fpermissive -I../lz4 -I../lzf -fPIC bitshuffle.c -c -o bitshuffle.o
	COMMAND g++ -shared -fpermissive -I../lz4 -I../lzf -fPIC bitshuffle_core.c -c -o bitshuffle_core.o
	COMMAND g++ -shared -fpermissive -I../lz4 -I../lzf -fPIC iochain.c -c -o iochain.o
	COMMAND g++ -shared -fpermissive -I../lz4 -I../lzf -fPIC ../lz4/lz4.c -c -o lz4.o
	COMMAND ar rcs ../libbitshuffle.a *.o
	WORKING_DIRECTORY ../modules/bitshuffle/src
)
add_custom_target(libbitshuffle DEPENDS ${LIBBITSHUFFLE_SO})
add_dependencies(streamwriter libbitshuffle)

# cbflib
set (LIBCBF_SO ${CMAKE_SOURCE_DIR}/modules/cbflib/CBFlib/solib/libcbf.so)
add_custom_command(
	OUTPUT ${LIBCBF_SO}
	COMMAND make shared
	WORKING_DIRECTORY ../modules/cbflib/CBFlib
)
add_custom_target(cbflib DEPENDS ${LIBCBF_SO})
add_dependencies(streamwriter cbflib)

# jsoncpp
set (JSONCPP_CPP ${CMAKE_SOURCE_DIR}/modules/jsoncpp/dist/jsoncpp.cpp)
add_custom_command(
	OUTPUT ${JSONCPP_CPP}
	COMMAND python amalgamate.py
	WORKING_DIRECTORY ../modules/jsoncpp
)
add_custom_target(jsoncpp DEPENDS ${JSONCPP_CPP})
add_dependencies(streamwriter jsoncpp)

# zeromq
set (LIBZMQ_SO ${CMAKE_SOURCE_DIR}/modules/libzmq/.libs/libzmq.so)
add_custom_command(
	OUTPUT ${LIBZMQ_SO}
	COMMAND ./autogen.sh
	COMMAND	./configure && make
	WORKING_DIRECTORY ../modules/libzmq
)
add_custom_target(libzmq DEPENDS ${LIBZMQ_SO})
add_dependencies(streamwriter libzmq)

# onion
set (LIBONION_SO ${CMAKE_SOURCE_DIR}/modules/onion/build/src/onion/libonion_static.a)
add_custom_command(
	OUTPUT ${LIBONION_SO}
	COMMAND mkdir -p ${CMAKE_SOURCE_DIR}/modules/onion/build
	COMMAND	cmake3 -Bbuild -H.
	COMMAND cmake3 --build build --target all
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/modules/onion
)
add_custom_target(libonion DEPENDS ${LIBONION_SO})
add_dependencies(streamwriter libonion)

# boost
set (LIBBOOST_SO ${CMAKE_SOURCE_DIR}/modules/boost/stage/lib/libboost_system.a)
add_custom_command(
	OUTPUT ${LIBBOOST_SO}
	COMMAND	./bootstrap.sh
	COMMAND ./b2 headers
	COMMAND ./b2 --with-regex --with-system --with-thread --with-log --with-filesystem
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/modules/boost
)
add_custom_target(libboost DEPENDS ${LIBBOOST_SO})
add_dependencies(streamwriter libboost)

# linking
set (LINK_LIBRARIES libonioncpp_static.a libonion_static.a libcbf.so libzmq.so libbitshuffle.a libboost_regex.a libboost_system.a libboost_thread.a libboost_log_setup.a libboost_log.a libboost_filesystem.a  )
target_link_libraries(streamwriter ${CMAKE_THREAD_LIBS_INIT} ${LINK_LIBRARIES})
