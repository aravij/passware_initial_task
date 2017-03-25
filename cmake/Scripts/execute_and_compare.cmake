set(HELP_MESSAGE "\
  This cmake script execute a given string in shell and compare its output.
  
  Usage: cmake -DEXEC_STRING=<EXEC_STRING_VALUE> -DDESIRED_OUTPUT=<DESIRED_OUTPUT_VALUE> -P execute_and_compare.cmake
  EXEC_STRING_VALUE   \tstring to be executed by the shell
  DESIRED_OUTPUT_VALUE\tstring to compare with output of EXEC_STRING
  
  The script exits normally, if outputs are equaled, and exits with fatal error, if not.
  The primary usage of the script is in functional test for checking targets outputs.\
")

if(NOT EXEC_STRING OR NOT DESIRED_OUTPUT)
    message(FATAL_ERROR
"  ERROR: Not all variable provided. Check script usage.
${HELP_MESSAGE}")
endif()

execute_process(COMMAND ${EXEC_STRING} RESULT_VARIABLE RESULT OUTPUT_VARIABLE OUTPUT ERROR_VARIABLE ERRORS)

if(RESULT)
    message(FATAL_ERROR
"${EXEC_STRING} exited with code ${RESULT} and following errors:
${ERRORS}")
endif()

if(ERRORS)
    message(WARNING
"  ${EXEC_STRING} exited normally, but still some error was printed.
  ${ERRORS}")
endif()

string(STRIP ${OUTPUT} OUTPUT)
if(NOT OUTPUT STREQUAL DESIRED_OUTPUT)
    message(FATAL_ERROR "The output ${OUTPUT} of ${EXEC_STRING} is differ from desired ${DESIRED_OUTPUT}.")
endif()
