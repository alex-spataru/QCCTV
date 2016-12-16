/*
 * Copyright (c) 2016 Alex Spataru
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE
 */

#include "QCCTV_CRC32.h"
#include "QCCTV_Communications.h"

static QCCTV_CRC32 crc32;

void QCCTV_InitStream (QCCTV_StreamPacket* packet)
{
    if (packet) {
        packet->fps = 18;
        packet->crc32 = 0;
        packet->cameraName = "Unknown";
        packet->cameraGroup = "Default";
        packet->flashlightEnabled = false;
        packet->resolution = QCCTV_Original;
        packet->autoRegulateResolution = true;
        packet->cameraStatus = QCCTV_CAMSTATUS_DEFAULT;
        packet->image = QCCTV_CreateStatusImage (QSize (640, 480),
                                                 "NO CAMERA IMAGE");
    }
}

void QCCTV_InitCommand (QCCTV_CommandPacket* command,
                        QCCTV_StreamPacket* stream)
{
    if (command && stream) {
        command->focusRequest = false;
        command->newFps = stream->fps;
        command->oldFps = stream->fps;
        command->oldResolution = stream->resolution;
        command->newResolution = stream->resolution;
        command->oldFlashlightEnabled = stream->flashlightEnabled;
        command->newFlashlightEnabled = stream->flashlightEnabled;
        command->oldAutoRegulateResolution = stream->autoRegulateResolution;
        command->newAutoRegulateResolution = stream->autoRegulateResolution;
    }
}

QByteArray QCCTV_CreateStreamPacket (const QCCTV_StreamPacket& packet)
{
    /* Create initial byte array */
    QByteArray data;

    /* Add camera name */
    data.append (packet.cameraName.length());
    data.append (packet.cameraName.toLatin1());

    /* Add camera group */
    data.append (packet.cameraGroup.length());
    data.append (packet.cameraGroup.toLatin1());

    /* Add FPS, light status and camera status */
    data.append ((quint8) packet.fps);
    data.append ((quint8) packet.flashlightEnabled);
    data.append ((quint8) packet.cameraStatus);
    data.append ((quint8) packet.resolution);
    data.append ((quint8) packet.autoRegulateResolution);

    /* Add raw image bytes */
    QCCTV_Resolution res = (QCCTV_Resolution) packet.resolution;
    QByteArray image = QCCTV_EncodeImage (packet.image, res);
    if (!image.isEmpty()) {
        data.append ((image.length() & 0xff0000) >> 16);
        data.append ((image.length() & 0xff00) >> 8);
        data.append ((image.length() & 0xff));
        data.append ((image));
    }

    /* Compress packet */
    data = qCompress (data, 9);

    /* Add the cheksum at the start of the data */
    quint32 crc = crc32.compute (data);
    data.prepend ((crc & 0xff));
    data.prepend ((crc & 0xff00) >> 8);
    data.prepend ((crc & 0xff0000) >> 16);
    data.prepend ((crc & 0xff000000) >> 24);

    /* Return obtained data */
    return data;
}

bool QCCTV_ReadStreamPacket (QCCTV_StreamPacket* packet,
                             const QByteArray& data)
{
    /* Packet pointer is invalid and/or data is empty */
    if (!packet || data.length() < 4)
        return false;

    /* Get the checksum */
    quint8 a = data.at (0);
    quint8 b = data.at (1);
    quint8 c = data.at (2);
    quint8 d = data.at (3);
    packet->crc32 = (a << 24) | (b << 16) | (c << 8) | (d & 0xff);

    /* Create byte array without checksum header */
    QByteArray stream = data;
    stream.remove (0, 4);

    /* Compare checksums (abort if they are different) */
    quint32 crc = crc32.compute (stream);
    if (packet->crc32 != crc)
        return false;

    /* Uncompress the stream data */
    int offset = 0;
    stream = qUncompress (stream);

    /* Get camera name */
    int name_len = stream.at (0);
    for (int i = 0; i < name_len; ++i) {
        int pos = 1 + i;
        if (stream.size() > pos)
            packet->cameraName.append (stream.at (pos));
        else
            return false;
    }

    /* Get camera group */
    int group_len = stream.at (name_len + 1);
    for (int i = 0; i < group_len; ++i) {
        int pos = name_len + 2 + i;
        if (stream.size() > pos)
            packet->cameraGroup.append (stream.at (pos));
        else
            return false;
    }

    /* Set offset value */
    offset = name_len + group_len + 1;

    /* Packet is too small to continue */
    if (stream.length() < offset + 5)
        return false;

    /* Get camera information  */
    else {
        packet->fps = (quint8) stream.at (offset + 1);
        packet->flashlightEnabled = (bool) stream.at (offset + 2);
        packet->cameraStatus = (int) stream.at (offset + 3);
        packet->resolution = (int) stream.at (offset + 4);
        packet->autoRegulateResolution = (bool) stream.at (offset + 5);
    }

    /* Packet is too small to get image length */
    if (stream.size() < offset + 8)
        return false;

    /* Get image length */
    quint8 img_a = stream.at (offset + 6);
    quint8 img_b = stream.at (offset + 7);
    quint8 img_c = stream.at (offset + 8);
    quint32 img_len = (img_a << 16) | (img_b << 8) | (img_c & 0xff);

    /* Get image bytes */
    QByteArray raw_image;
    for (quint32 i = 0; i < img_len; ++i) {
        int pos = offset + 9 + i;
        if (stream.size() > pos)
            raw_image.append (stream [pos]);
        else
            return false;
    }

    /* Try to decode the image */
    QImage img = QCCTV_DecodeImage (raw_image);
    if (!img.isNull())
        packet->image = img;
    else
        return false;

    /* Packet read successfully */
    return true;
}

QByteArray QCCTV_CreateCommandPacket (const QCCTV_CommandPacket& packet)
{
    QByteArray data;
    data.append ((quint8) packet.oldFps);
    data.append ((quint8) packet.newFps);
    data.append ((quint8) packet.oldResolution);
    data.append ((quint8) packet.newResolution);
    data.append ((quint8) packet.focusRequest ? 0x01 : 0x00);
    data.append ((quint8) packet.oldFlashlightEnabled ? 0x01 : 0x00);
    data.append ((quint8) packet.newFlashlightEnabled ? 0x01 : 0x00);
    data.append ((quint8) packet.oldAutoRegulateResolution ? 0x01 : 0x00);
    data.append ((quint8) packet.newAutoRegulateResolution ? 0x01 : 0x00);
    return data;
}

bool QCCTV_ReadCommandPacket (QCCTV_CommandPacket* packet,
                              const QByteArray& data)
{
    /* Packet pointer is invalid and/or data incomplete */
    if (!packet || data.length() < 9)
        return false;

    /* Get FPS */
    packet->oldFps = (quint8) data.at (0);
    packet->newFps = (quint8) data.at (1);

    /* Get resolution */
    packet->oldResolution = (quint8) data.at (2);
    packet->newResolution = (quint8) data.at (3);

    /* Get focus request */
    packet->focusRequest = (bool) data.at (4);

    /* Get flashlight enabled status */
    packet->oldFlashlightEnabled = (bool) data.at (5);
    packet->newFlashlightEnabled = (bool) data.at (6);

    /* Get auto-regulate resolution status */
    packet->oldAutoRegulateResolution = (bool) data.at (7);
    packet->newAutoRegulateResolution = (bool) data.at (8);

    /* Check command flags have changed since last packet */
    packet->fpsChanged = (packet->oldFps != packet->newFps);
    packet->resolutionChanged = (packet->oldResolution != packet->newResolution);
    packet->flashlightEnabledChanged = (packet->oldFlashlightEnabled !=
                                        packet->newFlashlightEnabled);
    packet->autoRegulateResolutionChanged = (packet->oldAutoRegulateResolution !=
                                             packet->newAutoRegulateResolution);

    /* Packet read successfully */
    return true;
}
