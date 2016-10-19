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

#include <QtQml>
#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include "QCCTV_Station.h"
#include "QCCTV_StationImage.h"

const QString APP_VERSION = "1.0";
const QString APP_COMPANY = "Alex Spataru";
const QString APP_DSPNAME = "QCCTV Station";
const QString APP_WEBSITE = "http://github.com/alex-spataru";

int main (int argc, char* argv[])
{
    /* Set application information */
    QGuiApplication::setApplicationName (APP_DSPNAME);
    QGuiApplication::setOrganizationName (APP_COMPANY);
    QGuiApplication::setApplicationVersion (APP_VERSION);
    QGuiApplication::setOrganizationDomain (APP_WEBSITE);

    /* Set application attributes */
    QGuiApplication::setAttribute (Qt::AA_UseDesktopOpenGL);
    QGuiApplication::setAttribute (Qt::AA_ShareOpenGLContexts);
    QGuiApplication::setAttribute (Qt::AA_EnableHighDpiScaling);

    /* Initialize application */
    QGuiApplication app (argc, argv);

    /* Initialize QCCTV station */
    QCCTV_Station station;
    QCCTV_StationImage provider (&station);

    /* Know if we are running on mobile or not */
#if defined Q_OS_ANDROID || defined Q_OS_IOS
    bool mobile = true;
#else
    bool mobile = false;
#endif

    /* Load QML interface */
    QQmlApplicationEngine engine;
    engine.addImageProvider ("qcctv", &provider);
    engine.rootContext()->setContextProperty ("isMobile", mobile);
    engine.rootContext()->setContextProperty ("QCCTVStation", &station);
    engine.rootContext()->setContextProperty ("AppDspName", APP_DSPNAME);
    engine.rootContext()->setContextProperty ("AppVersion", APP_VERSION);
    engine.load (QUrl (QStringLiteral ("qrc:/main.qml")));

    /* Exit if QML fails to load */
    if (engine.rootObjects().isEmpty())
        return EXIT_FAILURE;

    /* Enter application loop */
    return app.exec();
}

