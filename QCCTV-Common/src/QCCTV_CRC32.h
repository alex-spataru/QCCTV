/*
 * Copyright (c) 2016 Alex Spataru <alex_spataru@outlook.com>
 *
 * This file is part of the LibDS, which is released under the MIT license.
 * For more information, please read the LICENSE file in the root directory
 * of this project.
 */

#ifndef _QCCTV_CRC32_H
#define _QCCTV_CRC32_H

#include <QByteArray>

/**
 * \brief Computes the CRC32 data checksum of a data stream.
 *
 * Can be used to get the CRC32 over a stream if used with checked input/output
 * streams.
 */
class QCCTV_CRC32
{
public:
    explicit QCCTV_CRC32();

    quint32 compute (QByteArray buf);
    quint32 compute (QByteArray buf, int len);

private:
    quint32 crc_table [256];

private:
    void make_crc_table();
};


#endif
