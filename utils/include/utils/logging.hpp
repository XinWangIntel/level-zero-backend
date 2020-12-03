#ifndef LEVEL_ZERO_TESTS_LOGGING_HPP
#define LEVEL_ZERP_TESTS_LOGGING_HPP
#include <string>
#include <sstream>
#include <vector>
#include <iostream>
#include <string.h>

namespace level_zero_tests {
#define LOG_TRACE std::cout << std::endl 
#define LOG_DEBUG std::cout << std::endl
#define LOG_INFO  std::cout << std::endl
#define LOG_WARNING std::cout << std::endl
#define LOG_ERROR std::cout << std::endl
#define LOG_FATAL std::cout << std::endl

#define LOG_ENTER_FUNCTION std::cout << std::endl << "Enter function: " << __func__ << std::endl;
#define LOG_EXIT_FUNCTION std::cout << std::endl << "Exit function: " << __func__ << std::endl;
}

#endif
