add_library(nova_core 
    core/repository.cpp 
    core/logger.cpp 
    utils/fs_utils.cpp
    cli/parser.cpp
)
target_include_directories(nova_core PUBLIC ${CMAKE_SOURCE_DIR}/include)

add_executable(nova main.cpp)
target_link_libraries(nova PRIVATE nova_core)
