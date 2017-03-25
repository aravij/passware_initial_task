include(add_functional_test)

file(STRINGS functional_test_description TESTS)

foreach(TEST IN LISTS TESTS)
    STRING(REPLACE " " ";" TEST ${TEST})
    LIST(GET TEST 0 FILE)
    LIST(GET TEST 1 PASSWORD)
    add_functional_test(NAME finctional_test-${FILE}
                        COMMAND $<TARGET_FILE:test_problem> ${test_problem_SOURCE_DIR}/data/${FILE}
                        DESIRED_OUTPUT ${PASSWORD})

endforeach()
