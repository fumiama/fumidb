cmake_minimum_required(VERSION 3.0.0)
project(fumidb_test VERSION 1.0)

add_executable(binary_test binary_test.c)

add_test(test_binary binary_test COMMAND binary_test)