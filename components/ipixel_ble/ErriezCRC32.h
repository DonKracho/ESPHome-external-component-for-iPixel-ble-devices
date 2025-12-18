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
 * \file ErriezCRC32.h
 * \brief CRC32 library for Arduino
 * \details
 *      Source:         https://github.com/Erriez/ErriezCRC32
 *      Documentation:  https://erriez.github.io/ErriezCRC32
 */

#ifndef ERRIEZ_CRC32_H_
#define ERRIEZ_CRC32_H_

#include <inttypes.h> // uint32_t type
#include <stddef.h>   // size_t type

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \brief CRC32 initial value
 */
#define CRC32_INITIAL       0xFFFFFFFFUL

/*!
 * \brief CRC32 polynomial
 */
#define CRC32_POLYNOMIAL    0xEDB88320UL

uint32_t crc32String(const char *buffer);
uint32_t crc32Buffer(const void *buffer, size_t bufferLength);

uint32_t crc32Update(const void *buffer, size_t bufferLength, uint32_t crc);
uint32_t crc32Final(uint32_t crc);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // ERRIEZ_CRC32_H_
