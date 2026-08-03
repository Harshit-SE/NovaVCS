/**
 * @file error.hpp
 * @brief Exception classes for NovaVCS
 */
#pragma once
#include <stdexcept>
#include <string>

namespace nova::core {

/**
 * @class NovaException
 * @brief Base exception class for all NovaVCS specific errors.
 */
class NovaException : public std::runtime_error {
public:
    explicit NovaException(const std::string& msg) : std::runtime_error(msg) {}
};

} // namespace nova::core
