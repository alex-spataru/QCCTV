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
#include "QCCTV_Discovery.h"

/**
 * Initializes the class by connecting the signals/slots between the UDP
 * receiver sockets and the datagram handlers function of this class
 */
QCCTV_Discovery::QCCTV_Discovery()
{
    m_infoSocket.bind (QCCTV_INFO_PORT, QUdpSocket::ShareAddress);
    m_discoverySocket.bind (QCCTV_DISCOVERY_PORT, QUdpSocket::ShareAddress);

    connect (&m_infoSocket, SIGNAL (readyRead()), this, SLOT (readInfoPacket()));
    connect (&m_discoverySocket, SIGNAL (readyRead()), this, SLOT (readDiscoveryPacket()));
}

/**
 * Returns the only instance of the class
 */
QCCTV_Discovery* QCCTV_Discovery::getInstance()
{
    static QCCTV_Discovery instance;
    return &instance;
}

/**
 * Obtains the information datagram from a remote camera and notifies the
 * \a QCCTV_Station about the new packet
 */
void QCCTV_Discovery::readInfoPacket()
{
    while (m_infoSocket.hasPendingDatagrams()) {
        QByteArray data;
        QHostAddress address;
        data.resize (m_infoSocket.pendingDatagramSize());
        m_infoSocket.readDatagram (data.data(), data.size(), &address);

        emit newInfoPacket (QHostAddress (address.toIPv4Address()), data);
    }
}

/**
 * Obtains the remote host IP from which we received a packet, if the datagram
 * is valid, then the function will notify the rest of the QCCTV library
 */
void QCCTV_Discovery::readDiscoveryPacket()
{
    while (m_discoverySocket.hasPendingDatagrams()) {
        QByteArray data;
        QHostAddress address;
        data.resize (m_discoverySocket.pendingDatagramSize());
        int bytes = m_discoverySocket.readDatagram (data.data(), data.size(),
                                                    &address, NULL);

        if (bytes > 0)
            emit newCamera (QHostAddress (address.toIPv4Address()));
    }
}

