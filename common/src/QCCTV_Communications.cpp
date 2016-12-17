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

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

static QCCTV_CRC32 crc32;

/**
 * Initializes the default values for the given stream \a packet
 */
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

/**
 * Initializes the variables of the given \a command packet with the
 * values in the given \a stream packet
 */
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

/**
 * Reads the given stream \a packet and generates the binary data that can be
 * sent through a network socket to a connected QCCTV Station
 */
QByteArray QCCTV_CreateStreamPacket (const QCCTV_StreamPacket& packet)
{
    /* Create initial JSON object */
    QJsonObject json;

    /* Add information */
    json.insert ("fps", packet.fps);
    json.insert ("name", packet.cameraName);
    json.insert ("group", packet.cameraGroup);
    json.insert ("resolution", packet.resolution);
    json.insert ("cameraStatus", packet.cameraStatus);
    json.insert ("flashLightEnabled", packet.flashlightEnabled);
    json.insert ("autoRegulateResolution", packet.autoRegulateResolution);

    /* Add raw image bytes */
    QCCTV_Resolution res = (QCCTV_Resolution) packet.resolution;
    QByteArray image = QCCTV_EncodeImage (packet.image, res);
    if (!image.isEmpty())
        json.insert ("image", QString (image.toBase64()));

    /* Convert JSON data to binary data */
    QByteArray data = QJsonDocument (json).toBinaryData();

    /* Add the cheksum at the start of the data */
    quint32 crc = crc32.compute (data);
    data.prepend ((crc & 0xff));
    data.prepend ((crc & 0xff00) >> 8);
    data.prepend ((crc & 0xff0000) >> 16);
    data.prepend ((crc & 0xff000000) >> 24);

    /* Return obtained data */
    return data;
}

/**
 * Reads the given \a binary data and updates the values if the given stream
 * \a packet structure
 *
 * This function shall return \c true on success, \c false on failure
 */
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

    /* Get JSON object from data */
    QJsonObject json = QJsonDocument::fromBinaryData (stream).object();
    if (json.isEmpty())
        return false;

    /* Get information from JSON object */
    packet->fps = json.value ("fps").toInt();
    packet->cameraName = json.value ("name").toString();
    packet->cameraGroup = json.value ("group").toString();
    packet->resolution = json.value ("resolution").toInt();
    packet->cameraStatus = json.value ("cameraStatus").toInt();
    packet->flashlightEnabled = json.value ("flashlightEnabled").toBool();
    packet->autoRegulateResolution = json.value ("autoRegulateResolution").toBool();

    /* Get camera image from object */
    QByteArray base64 = json.value ("image").toString().toUtf8();
    if (!base64.isEmpty())
        packet->image = QCCTV_DecodeImage (QByteArray::fromBase64 (base64));

    /* Return the status of the image */
    return !packet->image.isNull();
}

/**
 * Reads the given command \a packet and generates the binary data that can be
 * sent through a network socket to a connected QCCTV Camera
 */
QByteArray QCCTV_CreateCommandPacket (const QCCTV_CommandPacket& packet)
{
    QJsonObject json;
    json.insert ("oldFps", packet.oldFps);
    json.insert ("newFps", packet.newFps);
    json.insert ("focusRequest", packet.focusRequest);
    json.insert ("oldResolution", packet.oldResolution);
    json.insert ("newResolution", packet.newResolution);
    json.insert ("oldFlashlightEnabled", packet.oldFlashlightEnabled);
    json.insert ("newFlashlightEnabled", packet.newFlashlightEnabled);
    json.insert ("oldAutoRegulateRes", packet.oldAutoRegulateResolution);
    json.insert ("newAutoRegulateRes", packet.newAutoRegulateResolution);
    return QJsonDocument (json).toBinaryData();
}

/**
 * Reads the given \a binary data and updates the values if the given command
 * \a packet structure
 *
 * This function shall return \c true on success, \c false on failure
 */
bool QCCTV_ReadCommandPacket (QCCTV_CommandPacket* packet,
                              const QByteArray& data)
{
    /* Packet pointer is invalid and/or data incomplete */
    if (!packet || data.isEmpty())
        return false;

    /* Get JSON object from data */
    QJsonObject json = QJsonDocument::fromBinaryData (data).object();
    if (json.isEmpty())
        return false;

    /* Get information from JSON object */
    packet->oldFps = json.value ("oldFps").toInt();
    packet->newFps = json.value ("newFps").toInt();
    packet->focusRequest = json.value ("focusRequest").toBool();
    packet->oldResolution = json.value ("oldResolution").toInt();
    packet->newResolution = json.value ("newResolution").toInt();
    packet->oldFlashlightEnabled = json.value ("oldFlashlightEnabled").toBool();
    packet->newFlashlightEnabled = json.value ("newFlashlightEnabled").toBool();
    packet->oldAutoRegulateResolution = json.value ("oldAutoRegulateRes").toBool();
    packet->newAutoRegulateResolution = json.value ("newAutoRegulateRes").toBool();

    /* Check command flags have changed since last packet */
    packet->fpsChanged = (packet->oldFps != packet->newFps);
    packet->resolutionChanged = (packet->oldResolution != packet->newResolution);
    packet->flashlightEnabledChanged = (packet->oldFlashlightEnabled != packet->newFlashlightEnabled);
    packet->autoRegulateResolutionChanged = (packet->oldAutoRegulateResolution !=
                                             packet->newAutoRegulateResolution);

    /* Packet read successfully */
    return true;
}
