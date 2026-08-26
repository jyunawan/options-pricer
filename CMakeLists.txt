cmake_minimum_required(VERSION 3.20)
project(options_pricer CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(OPTPRICER_WARNING_FLAGS -Wall -Wextra -pedantic-errors -Wconversion -Wsign-conversion -Werror)

add_library(optpricer INTERFACE)
target_include_directories(optpricer INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

include(CTest)
if(BUILD_TESTING)
  add_subdirectory(tests)
endif()

add_executable(price_cli apps/price_cli.cpp)
target_link_libraries(price_cli PRIVATE optpricer)
target_compile_options(price_cli PRIVATE ${OPTPRICER_WARNING_FLAGS})
