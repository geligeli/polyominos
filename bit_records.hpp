#pragma once

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <sys/types.h>

class BitRecordReaeder {
  public:
    explicit BitRecordReaeder(const std::filesystem::path& path);
    void seek(uint64_t bitIndex);
    void readBits(void* buffer, uint64_t num_bits);
  private:
    std::ifstream m_file;
    uint8_t m_bitIndexInByte;
    uint64_t m_byteIndex;
    char m_curByte;
    std::vector<uint8_t> m_buffer;
};