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

#include <QString>
#include <QHostAddress>

enum QCCTV_LightStatus {
    QCCTV_FLASHLIGHT_ON  = 0x01,
    QCCTV_FLASHLIGHT_OFF = 0x00,
};

enum QCCTV_CameraStatus {
    QCCTV_CAMSTATUS_DEFAULT       = 0b0,
    QCCTV_CAMSTATUS_CONNECTED     = 0b1,
    QCCTV_CAMSTATUS_LOW_BATTERY   = 0b10,
    QCCTV_CAMSTATUS_DISCHARING    = 0b100,
    QCCTV_CAMSTATUS_VIDEO_FAILURE = 0b1000,
    QCCTV_CAMSTATUS_LIGHT_FAILURE = 0b10000,
};

extern int QCCTV_GET_VALID_FPS (const int fps);
extern QString QCCTV_STATUS_STRING (const int status);

/* Additional command flags */
#define QCCTV_FORCE_FOCUS    0x05

/* Network ports */
#define QCCTV_STREAM_PORT    1100
#define QCCTV_COMMAND_PORT   1150
#define QCCTV_REQUEST_PORT   1200
#define QCCTV_DISCOVERY_PORT 1250

/* Network Addresses */
#define QCCTV_DISCOVERY_ADDR QHostAddress::Broadcast

/* Packet timings */
#define QCCTV_COMMAND_PKT_TIMING    500
#define QCCTV_REQUEST_PKT_TIMING    500
#define QCCTV_DISCVRY_PKT_TIMING    1000
#define QCCTV_CSTREAM_PKT_TIMING(x) 1000 / QCCTV_GET_VALID_FPS(x)

/* Packet watchdog timeout */
#define QCCTV_COMMAND_PKT_TIMEOUT 2000

/* Image encoding */
#define QCCTV_MIN_FPS            10
#define QCCTV_MAX_FPS            40
#define QCCTV_IMAGE_FORMAT       "PNG"
#define QCCTV_COMPRESSION_FACTOR 0x09
#define QCCTV_NO_IMAGE_FLAG      0x05

#endif

