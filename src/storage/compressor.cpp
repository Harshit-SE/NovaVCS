#include "nova/storage/compressor.hpp"
#include "nova/core/error.hpp"
#include <zlib.h>
#include <stdexcept>
#include <vector>

namespace nova::storage {

std::string Compressor::compress(const std::string& data) {
    z_stream zs;
    zs.zalloc = Z_NULL;
    zs.zfree = Z_NULL;
    zs.opaque = Z_NULL;
    
    if (deflateInit(&zs, Z_BEST_COMPRESSION) != Z_OK) {
        throw core::NovaException("deflateInit failed while compressing.");
    }
    
    zs.next_in = (Bytef*)data.data();
    zs.avail_in = data.size();
    
    int ret;
    char outbuffer[32768];
    std::string outstring;
    
    do {
        zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
        zs.avail_out = sizeof(outbuffer);
        ret = deflate(&zs, Z_FINISH);
        if (outstring.size() < zs.total_out) {
            outstring.append(outbuffer, zs.total_out - outstring.size());
        }
    } while (ret == Z_OK);
    
    deflateEnd(&zs);
    if (ret != Z_STREAM_END) {
        throw core::NovaException("Exception during zlib compression.");
    }
    
    return outstring;
}

std::string Compressor::decompress(const std::string& compressedData) {
    z_stream zs;
    zs.zalloc = Z_NULL;
    zs.zfree = Z_NULL;
    zs.opaque = Z_NULL;
    
    if (inflateInit(&zs) != Z_OK) {
        throw core::NovaException("inflateInit failed while decompressing.");
    }
    
    zs.next_in = (Bytef*)compressedData.data();
    zs.avail_in = compressedData.size();
    
    int ret;
    char outbuffer[32768];
    std::string outstring;
    
    do {
        zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
        zs.avail_out = sizeof(outbuffer);
        ret = inflate(&zs, 0);
        if (outstring.size() < zs.total_out) {
            outstring.append(outbuffer, zs.total_out - outstring.size());
        }
    } while (ret == Z_OK);
    
    inflateEnd(&zs);
    if (ret != Z_STREAM_END) {
        throw core::NovaException("Exception during zlib decompression. Stream corrupted.");
    }
    
    return outstring;
}

} // namespace nova::storage
