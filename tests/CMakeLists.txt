# Simple test executable
add_executable(test_runner test_main.cpp)
target_link_libraries(test_runner)

# Add test
add_test(NAME basic_test COMMAND test_runner)