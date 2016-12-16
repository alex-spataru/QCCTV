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

#ifndef _QCCTV_COMMUNICATIONS_H
#define _QCCTV_COMMUNICATIONS_H

#include "QCCTV.h"

struct QCCTV_StreamPacket {
    quint8 fps;
    QImage image;
    quint32 crc32;
    int resolution;
    int cameraStatus;
    QString cameraName;
    QString cameraGroup;
    bool flashlightEnabled;
    bool autoRegulateResolution;
};

struct QCCTV_CommandPacket {
    quint8 oldFps;
    quint8 newFps;
    bool focusRequest;
    quint8 oldResolution;
    quint8 newResolution;
    bool oldFlashlightEnabled;
    bool newFlashlightEnabled;
    bool oldAutoRegulateResolution;
    bool newAutoRegulateResolution;

    bool fpsChanged;
    bool resolutionChanged;
    bool flashlightEnabledChanged;
    bool autoRegulateResolutionChanged;
};

extern void QCCTV_InitStream (QCCTV_StreamPacket* packet);
extern void QCCTV_InitCommand (QCCTV_CommandPacket* command, QCCTV_StreamPacket* stream);
extern QByteArray QCCTV_CreateStreamPacket (const QCCTV_StreamPacket& packet);
extern bool QCCTV_ReadStreamPacket (QCCTV_StreamPacket* packet,
                                    const QByteArray& data);

extern QByteArray QCCTV_CreateCommandPacket (const QCCTV_CommandPacket& packet);
extern bool QCCTV_ReadCommandPacket (QCCTV_CommandPacket* packet,
                                     const QByteArray& data);

#endif
