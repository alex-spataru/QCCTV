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
#include "QCCTV_Station.h"

QCCTV_Station::QCCTV_Station() {}
QCCTV_Station::~QCCTV_Station() {}
QString QCCTV_Station::group() const {}
int QCCTV_Station::cameraCount() const {}
QCCTV_RemoteCamera* QCCTV_Station::getCamera (const int camera) {}
void QCCTV_Station::setCameraGroup (const QString& group) {}
void QCCTV_Station::setLightStatusAll (const QCCTV_LightStatus status) {}
void QCCTV_Station::setLightStatus (const int camera, const QCCTV_LightStatus status) {}
void QCCTV_Station::connectToCamera (const QHostAddress& ip) {}
