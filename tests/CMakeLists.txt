include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(nova_tests 
    unit/test_repository.cpp 
    unit/test_cli.cpp 
    unit/test_storage.cpp  
    test_index_manager.cpp
    test_branch_manager.cpp
    benchmarks/benchmark_diff.cpp
    core/merge_engine_test.cpp
    benchmarks/benchmark_search.cpp
)
target_link_libraries(nova_tests PRIVATE nova_core gtest_main)

include(GoogleTest)
gtest_discover_tests(nova_tests)
