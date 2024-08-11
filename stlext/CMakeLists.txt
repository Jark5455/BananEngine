set(CMAKE_CXX_STANDARD 20)

find_package(benchmark REQUIRED)

add_library(BananSTLExt SHARED cpuinfo.cpp wyhash.cpp)

add_executable(HashTest tests/hash_test.cpp)
add_executable(HashmapTest tests/hashmap_test.cpp)
add_executable(HashsetTest tests/hashset_test.cpp)

if (CMAKE_BUILD_TYPE MATCHES Debug)
    # target_link_libraries(HashTest PRIVATE BananSTLExt benchmark::benchmark coz::coz ${CMAKE_DL_LIBS})
    # target_link_libraries(HashmapTest PRIVATE BananSTLExt benchmark::benchmark coz::coz ${CMAKE_DL_LIBS})
    # target_link_libraries(HashsetTest PRIVATE BananSTLExt benchmark::benchmark coz::coz ${CMAKE_DL_LIBS})
else()
    target_link_libraries(HashTest PRIVATE BananSTLExt)
    target_link_libraries(HashmapTest PRIVATE BananSTLExt)
    target_link_libraries(HashsetTest PRIVATE BananSTLExt)
endif()