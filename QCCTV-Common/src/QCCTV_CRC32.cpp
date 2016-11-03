/*
 * Copyright (c) 2016 Alex Spataru <alex_spataru@outlook.com>
 *
 * This file is part of the LibDS, which is released under the MIT license.
 * For more information, please read the LICENSE file in the root directory
 * of this project.
 */

#include "QCCTV_CRC32.h"

/**
 * Generates the CRC table during initialization
 */
CRC32::CRC32()
{
    quint32 crc;

    for (int i = 0; i < 256; i++) {
        crc = i;
        for (int j = 0; j < 8; j++)
            crc = crc & 1 ? (crc >> 1) ^ 0xEDB88320UL : crc >> 1;

        crc_table[i] = crc;
    }
}

/**
 * Overloaded function, calculates the CRC of the given byte array as a whole
 *
 * @param buf the data buffer
 */
quint32 CRC32::compute (QByteArray buf)
{
    return compute (buf, buf.length());
}

/**
 * Adds the byte array to the data checksum.
 *
 * @param buf the data buffer
 * @param off the offset in the buffer where the data starts
 * @param len the length of the data
 */
quint32 CRC32::compute (QByteArray buf, int len)
{
    quint32 crc = 0xFFFFFFFFUL;

    for (int i = 0; i < len; ++i)
        crc = crc_table [ (crc ^ buf[i]) & 0xFF] ^ (crc >> 8);

    return crc ^ 0xFFFFFFFFUL;
}
