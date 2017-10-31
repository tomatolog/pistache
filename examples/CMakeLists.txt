function(pistache_example example_name)
    set(EXAMPLE_EXECUTABLE run_${example_name})
    set(EXAMPLE_SOURCE ${example_name}.cc)

    add_executable(${EXAMPLE_EXECUTABLE} ${EXAMPLE_SOURCE})
	link_directories (/rocksplicator/build/common)
	link_directories (/rocksplicator/build/common/stats)
	link_directories (/rocksplicator/build/rocksdb_replicator)
	link_directories (/rocksplicator/build/rocksdb_replicator/thrift)
	
	#target_link_libraries(${EXAMPLE_EXECUTABLE} pistache rocksdb rocksdb_replicator replicator_thrift jemalloc glog gflags folly thriftcpp2 thrift pthread stats boost_system common)
	#target_link_libraries(${EXAMPLE_EXECUTABLE} pistache rocksdb rocksdb_replicator replicator_thrift jemalloc glog folly thriftcpp2 thrift stats boost_system common /rocksplicator/build/common/jsoncpp/src/lib_json/libjsoncpp.a)
	#target_link_libraries(${EXAMPLE_EXECUTABLE} pistache rocksdb rocksdb_replicator replicator_thrift jemalloc glog folly thriftcpp2 thrift stats boost_system common)
	target_link_libraries(${EXAMPLE_EXECUTABLE} pistache rocksdb /rocksplicator/build/rocksdb_replicator/librocksdb_replicator.a /rocksplicator/build/rocksdb_replicator/thrift/libreplicator_thrift.a jemalloc glog gflags folly thriftcpp2 thrift /rocksplicator/build/common/stats/libstats.a boost_system /rocksplicator/build/common/libcommon.a wangle)
endfunction()

#find_package(gflags REQUIRED)
pistache_example(rest_server)

find_package(RapidJSON)
if (RapidJSON_FOUND)
include_directories(${RapidJSON_INCLUDE_DIRS})
pistache_example(rest_description)
endif()
