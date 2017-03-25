function(add_functional_test)
    cmake_parse_arguments(PARSE_ARGV 0 FUNCTIONAL_TEST "" "DESIRED_OUTPUT;NAME" COMMAND)
    
    find_file(EXECUTE_AND_COMPARE_SCRIPT execute_and_compare.cmake)
    
    separate_arguments(FUNCTIONAL_TEST_COMMAND UNIX_COMMAND "${FUNCTIONAL_TEST_COMMAND}")
    
    add_test(NAME ${FUNCTIONAL_TEST_NAME}
             COMMAND ${CMAKE_COMMAND}
                     -DEXEC_STRING=${FUNCTIONAL_TEST_COMMAND}
                     -DDESIRED_OUTPUT=${FUNCTIONAL_TEST_DESIRED_OUTPUT}
                     -P ${EXECUTE_AND_COMPARE_SCRIPT})
endfunction()
