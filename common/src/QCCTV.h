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
 * Flashlight status modes
 */
enum QCCTV_LightStatus {
    QCCTV_FLASHLIGHT_ON  = 0x01,
    QCCTV_FLASHLIGHT_OFF = 0x00,
};

/*
 * Camera status flags
 */
enum QCCTV_CameraStatus {
    QCCTV_CAMSTATUS_DEFAULT       = 0b0,
    QCCTV_CAMSTATUS_LOW_BATTERY   = 0b1,
    QCCTV_CAMSTATUS_DISCHARING    = 0b10,
    QCCTV_CAMSTATUS_VIDEO_FAILURE = 0b100,
    QCCTV_CAMSTATUS_LIGHT_FAILURE = 0b1000,
};

/*
 * Image resolutions
 */
enum QCCTV_Resolution {
    QCCTV_QCIF  = 0x00,
    QCCTV_CIF   = 0x01,
    QCCTV_2CIF  = 0x02,
    QCCTV_4CIF  = 0x03,
    QCCTV_D1    = 0x04,
    QCCTV_720p  = 0x05,
    QCCTV_960p  = 0x06,
    QCCTV_1080p = 0x07,
};

/*
 * Misc functions
 */
extern int QCCTV_WATCHDOG_TIME (const int fps);
extern int QCCTV_GET_VALID_FPS (const int fps);
extern QString QCCTV_STATUS_STRING (const int status);

/*
 * Image encoding/decoding functions
 */
extern QImage QCCTV_DECODE_IMAGE (const QByteArray& data);
extern QByteArray QCCTV_ENCODE_IMAGE (const QImage& image, QCCTV_Resolution res);

/*
 * Image generation function
 */
extern QStringList QCCTV_AVAILABLE_RESOLUTIONS();
extern QImage QCCTV_GET_STATUS_IMAGE (const QSize& size, const QString& text);

/*
 * Additional command flags
 */
#define QCCTV_FORCE_FOCUS 5

/*
 * Network ports
 */
#define QCCTV_STREAM_PORT    1100
#define QCCTV_COMMAND_PORT   1150
#define QCCTV_REQUEST_PORT   1200
#define QCCTV_DISCOVERY_PORT 1250

/*
 * Image encoding
 */
#define QCCTV_MIN_FPS      5
#define QCCTV_MAX_FPS      60
#define QCCTV_DEFAULT_FPS  18
#define QCCTV_IMAGE_FORMAT "JPG"

/*
 * Watchdog timings
 */
#define QCCTV_MIN_WATCHDOG_TIME 1000
#define QCCTV_MAX_WATCHDOG_TIME 5000

#endif

