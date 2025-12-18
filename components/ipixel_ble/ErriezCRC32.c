/*
 * MIT License
 *
 * Copyright (c) 2020 Erriez
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/*!
 * \file ErriezCRC32.c
 * \brief Flash size optimized CRC32 library for Arduino without using CRC tables
 * \details
 *      Source:         https://github.com/Erriez/ErriezCRC32
 *      Documentation:  https://erriez.github.io/ErriezCRC32
 */

#include <string.h> // Needed for strlen()

#include "ErriezCRC32.h"


/*!
 * \brief Calculate CRC on a character string.
 * \details
 *      This function is thread safe and maybe called from interrupt handlers.
 *      No buffer alignment needed.
 * \param buffer
 *      Null terminated character input array.
 * \return
 *      32-bit CRC value.
 *
 */
uint32_t crc32String(const char *buffer)
{
    return crc32Buffer(buffer, strlen(buffer));
}

/*!
 * \brief
 * \details
 *      This function is thread safe and maybe called from interrupt handlers.
 *      No buffer alignment needed.
 * \param buffer
 *      Input buffer.
 * \param bufferLength
 *      Buffer length in Bytes.
 * \return
 *      32-bit CRC value.
 */
uint32_t crc32Buffer(const void *buffer, size_t bufferLength)
{
    return ~crc32Update(buffer, bufferLength, CRC32_INITIAL);
}

/*!
 * \brief Calculate CRC on multiple buffers.
 * \details
 *      This function is thread safe and maybe called from interrupt handlers.
 *      No buffer alignment needed.
 * \param buffer
 *      Input buffer. No alignment needed.
 * \param bufferLength
 *      Input buffer length in Bytes. No alignment needed.
 * \param crc
 *      Previous CRC value. Call with crc = CRC32_INITIAL on first calculation.
 * \return
 *      32-bit updated CRC value.
 */
uint32_t crc32Update(const void *buffer, size_t bufferLength, uint32_t crc)
{
    size_t i;
    int8_t j;
    uint32_t mask;

    for (i = 0; i < bufferLength; i++) {
        crc ^= ((uint8_t *)buffer)[i];
        for (j = 7; j >= 0; j--) {
            mask = -(crc & 1);
            crc = (crc >> 1) ^ (CRC32_POLYNOMIAL & mask);
        }
    }

    return crc;
}

/*!
 * \brief Finalize CRC32 output
 * \details
 *      This function should be called after crc32Update().
 *      This function is thread safe and maybe called from interrupt handlers.
 *      No buffer alignment needed.
 * \param crc
 *      Previous CRC32 value.
 * \return
 *      32-bit CRC value.
 */
uint32_t crc32Final(uint32_t crc)
{
    return ~crc;
}
