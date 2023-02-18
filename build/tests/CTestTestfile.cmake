# CMake generated Testfile for 
# Source directory: /jeff/Projects/jinject/tests
# Build directory: /jeff/Projects/jinject/build/tests
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(sample_test "sample_test" "COMMAND" "$<TARGET_FILE:sample_test>")
set_tests_properties(sample_test PROPERTIES  _BACKTRACE_TRIPLES "/jeff/Projects/jinject/tests/CMakeLists.txt;23;add_test;/jeff/Projects/jinject/tests/CMakeLists.txt;36;module_test;/jeff/Projects/jinject/tests/CMakeLists.txt;0;")
subdirs("../_deps/googletest-build")
