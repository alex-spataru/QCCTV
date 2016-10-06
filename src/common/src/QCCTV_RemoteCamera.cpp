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

#include "QCCTV.h"
#include "QCCTV_RemoteCamera.h"

QCCTV_RemoteCamera::QCCTV_RemoteCamera() {}
QCCTV_RemoteCamera::~QCCTV_RemoteCamera() {}
int QCCTV_RemoteCamera::fps() const {}
QString QCCTV_RemoteCamera::group() const {}
QString QCCTV_RemoteCamera::cameraName() const {}
QImage QCCTV_RemoteCamera::currentImage() const {}
QHostAddress QCCTV_RemoteCamera::address() const {}
QCCTV_LightStatus QCCTV_RemoteCamera::lightStatus() const {}
QCCTV_CameraStatus QCCTV_RemoteCamera::cameraStatus() const {}
void QCCTV_RemoteCamera::setFPS (const int fps) {}
void QCCTV_RemoteCamera::attemptConnection (const QHostAddress& address) {}
void QCCTV_RemoteCamera::setLightStatus (const QCCTV_LightStatus status) {}
void QCCTV_RemoteCamera::setCameraStatus (const QCCTV_CameraStatus status) {}
