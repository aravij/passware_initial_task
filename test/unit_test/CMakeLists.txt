add_executable(unit_test unit_test.cpp cartesian_range_power_test.cpp
                         parse_file_test.cpp
                         $<TARGET_PROPERTY:test_problem,SOURCE_DIR>/io/parse_file.cpp)

find_package(Boost COMPONENTS unit_test_framework REQUIRED)

target_include_directories(unit_test PUBLIC ${Boost_INCLUDE_DIRS}
                                            $<TARGET_PROPERTY:test_problem,SOURCE_DIR>
                                            $<TARGET_PROPERTY:test_problem,INCLUDE_DIRECTORIES>)
target_link_libraries(unit_test ${Boost_LIBRARIES} $<TARGET_PROPERTY:test_problem,LINK_LIBRARIES>)
target_compile_definitions(unit_test PUBLIC $<TARGET_PROPERTY:test_problem,COMPILE_DEFINITIONS>)

set_property(TARGET unit_test PROPERTY CXX_STANDARD 14)
set_property(TARGET unit_test PROPERTY CXX_STANDARD_REQUIRED ON)

add_test(NAME unit_test COMMAND unit_test)
