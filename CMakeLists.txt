cmake_minimum_required(VERSION 3.16)
project(itchbook VERSION 0.1.0 LANGUAGES CXX)

add_library(itchbook_core src/itchbook.cpp)
target_include_directories(itchbook_core PUBLIC include)
target_compile_features(itchbook_core PUBLIC cxx_std_17)
set_target_properties(itchbook_core PROPERTIES CXX_EXTENSIONS OFF)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(itchbook_core PRIVATE -Wall -Wextra -Wpedantic -Wshadow -Werror)
endif()

add_executable(itchbook src/main.cpp)
target_link_libraries(itchbook PRIVATE itchbook_core)

include(CTest)
if(BUILD_TESTING)
  add_executable(itchbook_tests tests/test_itchbook.cpp)
  target_link_libraries(itchbook_tests PRIVATE itchbook_core)
  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(itchbook_tests PRIVATE -Wall -Wextra -Wpedantic -Wshadow -Werror)
  endif()
  add_test(NAME replay_contract COMMAND itchbook_tests)
endif()
