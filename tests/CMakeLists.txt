include(FetchContent)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.15.3
)
FetchContent_MakeAvailable(Catch2)

add_executable(optpricer_tests
  test_black_scholes.cpp
  test_parity.cpp
)
target_link_libraries(optpricer_tests PRIVATE optpricer Catch2::Catch2WithMain)
target_compile_options(optpricer_tests PRIVATE ${OPTPRICER_WARNING_FLAGS})

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(Catch)
catch_discover_tests(optpricer_tests)
