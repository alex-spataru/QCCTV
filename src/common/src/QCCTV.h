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

enum QCCTV_LightStatus {
    QCCTV_LIGHT_OFF        = 0x01,
    QCCTV_LIGHT_SLOW_BLINK = 0x02,
    QCCTV_LIGHT_FAST_BLINK = 0x03,
    QCCTV_LIGHT_CONTINOUS  = 0x04,
};

enum QCCTV_CameraStatus {
    QCCTV_CAMSTATUS_OK            = 0x01,
    QCCTV_CAMSTATUS_DISCONNECTED  = 0x02,
    QCCTV_CAMSTATUS_LOW_BATTERY   = 0x03,
    QCCTV_CAMSTATUS_DISCHARING    = 0x04,
    QCCTV_CAMSTATUS_VIDEO_FAILURE = 0x05,
    QCCTV_CAMSTATUS_LIGHT_FAILURE = 0x06,
};


#define QCCTV_DATA_SEPARATOR 0x05

#define QCCTV_STREAM_PORT 15400
#define QCCTV_COMMAND_PORT 25400
#define QCCTV_REQUEST_PORT 25300
#define QCCTV_DISCOVERY_PORT 54000

#endif

