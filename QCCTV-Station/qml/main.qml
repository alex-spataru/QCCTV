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

import QtQuick 2.0
import QtMultimedia 5.4
import QtQuick.Controls 2.0
import QtQuick.Controls.Universal 2.0

import "."

ApplicationWindow {
    id: app
    
    //
    // Global variables
    //
    property int spacing: 12
    property string fontFamily: "OpenSans"

    //
    // Window geometry
    //
    width: 840
    height: 520
    
    //
    // Window properties
    //
    color: "#000"
    visible: true
    Universal.theme: Universal.Dark
    title: AppDspName + " " + AppVersion

    //
    // QCCTV signals/slots
    //
    Connections {
        target: QCCTVStation
        onCameraCountChanged: {
            grid.model = QCCTVStation.cameraCount()
            noCameras.opacity = QCCTVStation.cameraCount() > 0 ? 0 : 1
        }
    }

    //
    // Camera grid
    //
    GridView {
        id: grid
        model: QCCTVStation.cameraCount()
        delegate: Camera {
            camera: index
        }
    }

    //
    // No Cameras Labels
    //
    Column {
        id: noCameras
        spacing: app.spacing
        anchors.centerIn: parent
        opacity: QCCTVStation.cameraCount() > 0 ? 0 : 1

        Behavior on opacity { NumberAnimation{} }

        Label {
            color: "#fff"
            font.bold: true
            font.pixelSize: 24
            font.family: app.fontFamily
            text: qsTr ("No QCCTV Cameras Found")
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Label {
            color: "#ccc"
            font.pixelSize: 16
            font.family: app.fontFamily
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr ("Try launching QCCTV Camera in some of your LAN devices")
        }
    }
}
