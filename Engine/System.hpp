#pragma once

#include <cstdint>
#include <utility>
#include <vector>

class System {
public:
    /// Given a filename, returns the data contained within the file along with its relative size.
    static std::pair<const char*, long> readFile(const char* pFilename);
};