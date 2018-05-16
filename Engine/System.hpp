#pragma once

#ifdef _WIN32
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#endif

#include <cstdint>
#include <utility>
#include <vector>

class System {
public:
    /// Given a filename, returns the data contained within the file along with its relative size.
    static std::pair<const char*, long> readFile(const char* pFilename);
};