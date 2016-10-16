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
#include <QCamera>
#include <QQuickStyle>
#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include "QCCTV_LocalCamera.h"

const QString APP_VERSION = "1.0";
const QString APP_COMPANY = "Alex Spataru";
const QString APP_DSPNAME = "QCCTV Camera";
const QString APP_WEBSITE = "http://github.com/alex-spataru";

int main (int argc, char* argv[])
{
    /* Set application information */
    QGuiApplication::setApplicationName (APP_DSPNAME);
    QGuiApplication::setOrganizationName (APP_COMPANY);
    QGuiApplication::setApplicationVersion (APP_VERSION);
    QGuiApplication::setOrganizationDomain (APP_WEBSITE);
    QGuiApplication::setAttribute (Qt::AA_EnableHighDpiScaling);

    /* Initialize application */
    QGuiApplication app (argc, argv);
    QQuickStyle::setStyle ("Universal");

    /* Initialize QCCTV camera */
    QCCTV_LocalCamera camera;

    /* Load QML interface */
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty ("QCCTVCamera", &camera);
    engine.rootContext()->setContextProperty ("AppDspName", APP_DSPNAME);
    engine.rootContext()->setContextProperty ("AppVersion", APP_VERSION);
    engine.load (QUrl (QStringLiteral ("qrc:/main.qml")));

    /* Obtain camera from QML UI */
    QObject* qmlCam = engine.rootObjects().at (0)->findChild<QObject*> ("camera");
    camera.setCamera (qvariant_cast<QCamera*> (qmlCam->property ("mediaObject")));

    /* Exit if QML fails to load */
    if (engine.rootObjects().isEmpty())
        return EXIT_FAILURE;

    /* Enter application loop */
    return app.exec();
}

