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
#include "QCCTV_RemoteCamera.h"

/**
 * Initializes the class by connecting the signals/slots between the UDP
 * receiver socket and the datagram handler function of this class
 */
QCCTV_Discovery::QCCTV_Discovery() {
    m_socket.bind (QHostAddress::Any,
                   QCCTV_DISCOVERY_PORT,
                   QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);

    connect (&m_socket, SIGNAL (readyRead()), this, SLOT (readPacket()));
}

/**
 * Returns the only instance of the class
 */
QCCTV_Discovery* QCCTV_Discovery::getInstance() {
    static QCCTV_Discovery instance;
    return &instance;
}

/**
 * Obtains the remote host IP from which we received a packet, if the datagram
 * is valid, then the function will notify the rest of the QCCTV library
 */
void QCCTV_Discovery::readPacket() {
    while (m_socket.hasPendingDatagrams()) {
        int error;
        QHostAddress ip;
        QByteArray array;

        array.resize (m_socket.pendingDatagramSize());
        error = m_socket.readDatagram (array.data(), array.size(), &ip, NULL);

        if (error > 0)
            emit newCamera (ip);
    }
}

