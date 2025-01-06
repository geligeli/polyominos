#include "bit_records.hpp"
#include <cstring>
#include <fcntl.h>
#include <ios>
#include <unistd.h>

BitRecordReaeder::BitRecordReaeder(const std::filesystem::path &path)
    : m_file(path, std::ios_base::binary), m_bitIndexInByte(0), m_byteIndex(0) {
}

void BitRecordReaeder::seek(uint64_t bitIndex) {
  m_byteIndex = bitIndex / 8;
  m_bitIndexInByte = bitIndex % 8;
  m_file.seekg(m_byteIndex, std::ios_base::beg);
  if (m_bitIndexInByte != 0) {
    m_file.read(&m_curByte, 1);
  }
}

/*
00000VVV VVVVVVVV VVVVV000
         |
         current read position

*/
void BitRecordReaeder::readBits(void* buffer, uint64_t num_bits) {
    uint8_t* byteBuffer = static_cast<uint8_t*>(buffer);
    
    uint64_t numBitsInBuffer = 8-m_bitIndexInByte;

    if (numBitsInBuffer >= num_bits) {
        uint8_t mask = (1 << num_bits) - 1;
        uint8_t byte = m_curByte >> m_bitIndexInByte;
        byte &= mask;
        byteBuffer[0] = byte;
        m_bitIndexInByte += num_bits;
        return;
    }

    m_buffer.resize(num_bits / 8 + 1);

    m_file.read(reinterpret_cast<char*>(m_buffer.data()), num_bits / 8 + 1);

    uint64_t remainingBitsToRead = num_bits - numBitsInBuffer;
    uint64_t numBytes = remainingBitsToRead / 8;
    uint64_t numBitsInLastByte = remainingBitsToRead % 8;

    if (numBitsInLastByte == 0) {
        m_file.read(reinterpret_cast<char*>(byteBuffer), numBytes);
        return;
    }

    // if (numBitsInLastByte == 0) {
    //     m_file.read(reinterpret_cast<char*>(byteBuffer), numBytes);
    //     return;
    // }
    // m_file.read(reinterpret_cast<char*>(byteBuffer), numBytes);
    // byteBuffer[numBytes] = 0;
    // m_file.read(reinterpret_cast<char*>(&byteBuffer[numBytes]), 1);
    // byteBuffer[numBytes] >>= (8 - numBitsInLastByte);
}
