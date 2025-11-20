#pragma once

#include <vector>
#include <string>
#include <ErriezCRC32.h>

namespace Helpers {

    //Reverse byte order of vector
    std::vector<uint8_t> switchEndian(const std::vector<uint8_t>& bytes);

    //Reverse bits in a 16-bit word
    uint16_t reverseBits16(uint16_t n);

    //Reverse bits in each 16-bit chunk of vector
    std::vector<uint8_t> logicReverseBitsOrder(const std::vector<uint8_t>& data);

    //Reverse order of 2-byte frames
    std::vector<uint8_t> invertFrames(const std::vector<uint8_t>& data);

    //Compute CRC32 as 4-byte vector, reversed
    std::vector<uint8_t> calculateCRC32Bytes(const std::vector<uint8_t>& data);

    //Returns the frame size in a vector of size bytes
    std::vector<uint8_t> getFrameSize(const std::vector<uint8_t>& data, size_t size);

    //Turns a hex string into a vector
    std::vector<uint8_t> hexStringToVector(const std::string &hexString);

    //Coverts UTF8 string characters to bitmaps
    bool encodeText(const std::string& text, int font_flag, uint8_t r, uint8_t g, uint8_t b, uint &length, std::vector<uint8_t> &frame);
}