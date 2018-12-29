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

#include "QCCTV_Watchdog.h"

QCCTV_Watchdog::QCCTV_Watchdog (QObject* parent) : QObject (parent)
{
    m_timer = new QTimer (this);
    connect (m_timer, SIGNAL (timeout()), this, SIGNAL (expired()));
}

/**
 * Returns the expiration time of the watchdog in milliseconds
 */
int QCCTV_Watchdog::expirationTime() const
{
    return m_timer->interval();
}

/**
 * Resets the watchdog and prevents it from expiring
 */
void QCCTV_Watchdog::reset()
{
    m_timer->stop();
    m_timer->start (expirationTime());
}

/**
 * Changes the expiration time and resets the watchdog
 */
void QCCTV_Watchdog::setExpirationTime (const int time)
{
    m_timer->setInterval (time);
    reset();
}
