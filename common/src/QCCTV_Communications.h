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

struct QCCTV_InfoPacket {
    quint8 fps;
    quint8 zoom;
    int resolution;
    int cameraStatus;
    bool supportsZoom;
    QString cameraName;
    QString cameraGroup;
    bool flashlightEnabled;
    bool autoRegulateResolution;
};

struct QCCTV_ImagePacket {
    QImage image;
    quint32 crc32;
};

struct QCCTV_CommandPacket {
    QString host;
    quint8 oldFps;
    quint8 newFps;
    quint8 oldZoom;
    quint8 newZoom;
    bool focusRequest;
    quint8 oldResolution;
    quint8 newResolution;
    bool oldFlashlightEnabled;
    bool newFlashlightEnabled;
    bool oldAutoRegulateResolution;
    bool newAutoRegulateResolution;

    bool fpsChanged;
    bool zoomChanged;
    bool resolutionChanged;
    bool flashlightEnabledChanged;
    bool autoRegulateResolutionChanged;
};


extern void QCCTV_InitInfo (QCCTV_InfoPacket* packet);
extern void QCCTV_InitImage (QCCTV_ImagePacket* packet);
extern void QCCTV_InitCommand (QCCTV_CommandPacket* command, QCCTV_InfoPacket* stream);

extern void QCCTV_WriteImagePacket (QByteArray* out,
                                    const QCCTV_ImagePacket* image,
                                    const QCCTV_InfoPacket* info);

extern QByteArray QCCTV_CreateInfoPacket (const QCCTV_InfoPacket* packet);
extern QByteArray QCCTV_CreateCommandPacket (const QCCTV_CommandPacket* packet);
extern QByteArray QCCTV_CreateImagePacket (const QCCTV_ImagePacket* packet,
                                           const QCCTV_InfoPacket* info);

extern bool QCCTV_ReadInfoPacket (QCCTV_InfoPacket* packet, const QByteArray& data);
extern bool QCCTV_ReadImagePacket (QCCTV_ImagePacket* packet, const QByteArray& data);
extern bool QCCTV_ReadCommandPacket (QCCTV_CommandPacket* packet, const QByteArray& data);

#endif
