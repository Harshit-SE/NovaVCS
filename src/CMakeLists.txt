find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)

add_library(nova_core 
    core/repository.cpp 
    core/logger.cpp 
    utils/fs_utils.cpp
    cli/parser.cpp
    crypto/sha256.cpp
    storage/compressor.cpp
    storage/object_db.cpp
    IndexManager.cpp
    core/commit.cpp
    core/commit_graph.cpp
    core/branch_manager.cpp
    core/diff_engine.cpp
    core/merge_engine.cpp
    core/search_engine.cpp
    compression/compression_engine.cpp
)


target_include_directories(nova_core PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(nova_core PUBLIC OpenSSL::Crypto ZLIB::ZLIB)

add_executable(nova main.cpp)
target_link_libraries(nova PRIVATE nova_core)
