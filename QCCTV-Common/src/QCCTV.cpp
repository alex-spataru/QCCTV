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

#include <QObject>

/**
 * If a is not empty, the function appends \a b to \a a and adds a separator.
 * Otherwise, this function shall return \a b
 */
static QString append_str (const QString a, QString b)
{
    if (a.isEmpty())
        return b;

    else
        return a + " | " + b;
}

/**
 * Returns a valid FPS value
 */
int QCCTV_GET_VALID_FPS (const int fps)
{
    return qMax (qMin (fps, QCCTV_MAX_FPS), QCCTV_MIN_FPS);
}

/**
 * Parses the given status flags as a string
 */
QString QCCTV_STATUS_STRING (const int status)
{
    QString str;

    if (status & QCCTV_CAMSTATUS_CONNECTED)
        str = append_str (str, QObject::tr ("Connected"));
    else
        str = append_str (str, QObject::tr ("Disconnected"));

    if (status & QCCTV_CAMSTATUS_LOW_BATTERY)
        str = append_str (str, QObject::tr ("Low Battery"));

    if (status & QCCTV_CAMSTATUS_DISCHARING)
        str = append_str (str, QObject::tr ("Discharging"));

    if (status & QCCTV_CAMSTATUS_LIGHT_FAILURE)
        str = append_str (str, QObject::tr ("Flashlight Failure"));

    if (status & QCCTV_CAMSTATUS_VIDEO_FAILURE)
        str = append_str (str, QObject::tr ("Video Failure"));

    return str;
}
