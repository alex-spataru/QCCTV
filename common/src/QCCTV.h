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

#ifndef _QCCTV_GLOBAL_H
#define _QCCTV_GLOBAL_H

#include <QImage>
#include <QString>
#include <QHostAddress>

/*
 * Set network ports
 */
#define QCCTV_INFO_PORT      1050
#define QCCTV_STREAM_PORT    1100
#define QCCTV_COMMAND_PORT   1150
#define QCCTV_REQUEST_PORT   1200
#define QCCTV_DISCOVERY_PORT 1250

/*
 * Image encoding
 */
#define QCCTV_MIN_FPS         5
#define QCCTV_MAX_FPS         30
#define QCCTV_MAX_BUFFER_SIZE 250 * 1024
#define QCCTV_RECORDINGS_PATH QDir::homePath() + "/Documents/QCCTV/"

/*
 * Watchdog timings
 */
#define QCCTV_MIN_WATCHDOG_TIME 500
#define QCCTV_MAX_WATCHDOG_TIME 3000

/*
 * Ugly OS fixes
 */
#if defined (Q_OS_LINUX) || defined (Q_OS_ANDROID)
    #define QCCTV_USE_FALLBACK_INTERFACE
#else
    #ifdef QCCTV_USE_FALLBACK_INTERFACE
        #undef QCCTV_USE_FALLBACK_INTERFACE
    #endif
#endif

/*
 * Camera status flags
 */
enum QCCTV_CameraStatus {
    QCCTV_CAMSTATUS_DEFAULT       = 0b0,
    QCCTV_CAMSTATUS_VIDEO_FAILURE = 0b10,
    QCCTV_CAMSTATUS_LIGHT_FAILURE = 0b100,
};

/*
 * Image resolutions
 */
enum QCCTV_Resolution {
    QCCTV_QCIF     = 0x00,
    QCCTV_CIF      = 0x01,
    QCCTV_2CIF     = 0x02,
    QCCTV_4CIF     = 0x03,
    QCCTV_D1       = 0x04,
    QCCTV_720p     = 0x05,
    QCCTV_960p     = 0x06,
    QCCTV_Original = 0x07
};

/*
 * Misc functions
 */
extern QStringList QCCTV_Resolutions();
extern int QCCTV_ValidFps (const int fps);
extern int QCCTV_GetWatchdogTime (const int fps);
extern QSize QCCTV_GetResolution (const int resolution);
extern QString QCCTV_GetStatusString (const int status);
extern QImage QCCTV_DecodeImage (const QByteArray& data);
extern QByteArray QCCTV_EncodeImage (const QImage& image, const int res);
extern QImage QCCTV_CreateStatusImage (const QSize& size, const QString& text);

#endif

