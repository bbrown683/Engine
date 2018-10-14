#include "System.hpp"

std::pair<const char*, long> System::readFile(const char* pFilename) {
    // Uses the C API for reading files.
    // MSVC wants use to use the secure alternative.
    FILE* file;
    file = fopen(pFilename, "rb");

    if (!file)
        return std::make_pair<const char*, long>('\0', 0);
    // Get the size of the file so we can preallocate 
    // the correct amount for the vector.
    long pos = ftell(file);
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, pos, SEEK_SET);

    std::vector<char> buffer(size + 1);
    fread(buffer.data(), size, 1, file);
    buffer[size] = 0;
    fclose(file);
    return std::make_pair(buffer.data(), size);
}
