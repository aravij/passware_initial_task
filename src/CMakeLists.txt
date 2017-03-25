add_executable(test_problem main.cpp io/parse_file.cpp cryptography/check_password.cpp)

find_package(Boost REQUIRED COMPONENTS program_options filesystem)
find_package(GCrypt REQUIRED)

target_include_directories(test_problem PUBLIC ${Boost_INCLUDE_DIRS} ${GCRYPT_INCLUDE_DIRS})
target_link_libraries(test_problem ${Boost_LIBRARIES} ${GCRYPT_LIBRARIES})
target_compile_definitions(test_problem PUBLIC ${GCRYPT_DEFINITIONS})

find_package(OpenMP)
if(${OpenMP_FOUND})
    # target_compile_options(test_problem PUBLIC ${OpenMP_CXX_FLAGS})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
else()
    message(STATUS "OpenMP wasn't found. Building program without multithreading.")
endif()

set_property(TARGET test_problem PROPERTY CXX_STANDARD 14)
set_property(TARGET test_problem PROPERTY CXX_STANDARD_REQUIRED ON)
