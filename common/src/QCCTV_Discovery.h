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

#ifndef _QCCTV_DISCOVERY_H
#define _QCCTV_DISCOVERY_H

#include <QObject>
#include <QUdpSocket>
#include <QHostAddress>

class QCCTV_Discovery : public QObject
{
    Q_OBJECT

Q_SIGNALS:
    void newCamera (const QHostAddress& camera);
    void newInfoPacket (const QHostAddress& camera, const QByteArray& data);

public:
    static QCCTV_Discovery* getInstance();

private Q_SLOTS:
    void readInfoPacket();
    void readDiscoveryPacket();

protected:
    QCCTV_Discovery();

private:
    QUdpSocket m_infoSocket;
    QUdpSocket m_discoverySocket;
};

#endif

