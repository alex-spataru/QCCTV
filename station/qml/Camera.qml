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
import QtQuick.Layouts 1.0
import QtQuick.Controls 2.0

Item {
    id: cam

    //
    // Signals
    //
    signal clicked

    //
    // Properties
    //
    property int camNumber: 0
    property string cameraName: ""
    property bool controlsEnabled: !returnButtonEnabled
    property bool returnButtonEnabled: QCCTVStation.cameraCount() > 1

    //
    // Obtains latest camera data from QCCTV
    //
    function reloadData() {
        fpsSpinbox.value = QCCTVStation.fps (camNumber)
        cameraName = QCCTVStation.cameraName (camNumber)
        resolutions.currentIndex = QCCTVStation.resolution (camNumber)
        lightButton.flashOn = QCCTVStation.flashlightEnabled (camNumber)

        image.source = "image://qcctv/reload/" + camNumber
        image.source = "image://qcctv/" + camNumber
        image.sourceChanged (image.source)

        minimalTitle.text = cameraName
    }

    //
    // Load initial camera data during creation
    //
    Component.onCompleted: reloadData()

    //
    // React to QCCTV events
    //
    Connections {
        enabled: true
        target: QCCTVStation

        onNewCameraImage: {
            if (camera === camNumber) {
                image.source = "image://qcctv/reload/" + camNumber
                image.source = "image://qcctv/" + camNumber
                image.sourceChanged (image.source)
            }
        }

        onFpsChanged: {
            if (camera === camNumber)
                fpsSpinbox.value = QCCTVStation.fps (camNumber)
        }

        onLightStatusChanged: {
            if (camera === camNumber)
                lightButton.flashOn = QCCTVStation.flashlightEnabled (camNumber)
        }

        onCameraNameChanged: {
            if (camera === camNumber) {
                cameraName = QCCTVStation.cameraName (camNumber)
                minimalTitle.text = QCCTVStation.cameraName (camNumber)
            }
        }

        onResolutionChanged: {
            if (camera === camNumber)
                resolutions.currentIndex = QCCTVStation.resolution (camNumber)
        }
    }

    //
    // Camera image
    //
    Image {
        id: image
        cache: false
        asynchronous: true
        anchors.fill: parent
        fillMode: Image.PreserveAspectCrop
    }

    //
    // Minimal title label (shown in the grid layout)
    //
    Label {
        id: minimalTitle
        text: cameraName
        font.pixelSize: Math.min (14, image.height / 12)

        anchors {
            top: parent.top
            left: parent.left
            margins: app.spacing
        }

        opacity: controlsEnabled ? 0 : 1
    }

    //
    // Allow user to enlarge camera image to fill app screen
    //
    MouseArea {
        anchors.fill: parent
        onClicked: cam.enabled ? cam.clicked() : undefined
    }

    //
    // FPS setter dialog
    //
    Popup {
        id: fpsDialog

        modal: true
        x: (parent.width - width) / 2
        y: (parent.height - height) / 2

        Connections {
            target: QCCTVStation
            onCameraCountChanged: {
                if (QCCTVStation.cameraCount() < 1 && visible)
                    fpsDialog.close()
            }
        }

        ColumnLayout {
            spacing: app.spacing
            anchors.centerIn: parent
            anchors.margins: app.spacing

            RowLayout {
                spacing: app.spacing
                Layout.fillHeight: true

                Image {
                    sourceSize: Qt.size (64, 64)
                    source: "qrc:/images/update.svg"
                }

                ColumnLayout {
                    spacing: app.spacing
                    Layout.fillWidth: true

                    Label {
                        text: qsTr ("Set target FPS") + ":"
                    }

                    SpinBox {
                        id: fpsSpinbox
                        Layout.fillWidth: true
                        Layout.minimumWidth: 180
                        to: QCCTVStation.maximumFPS()
                        from: QCCTVStation.minimumFPS()
                        onValueChanged: QCCTVStation.changeFPS (camNumber, value)
                    }
                }
            }

            Button {
                text: qsTr ("Close")
                Layout.fillWidth: true
                onClicked: fpsDialog.close()
            }
        }
    }

    //
    // Camera Controls
    //
    RowLayout {
        spacing: app.spacing
        visible: controlsEnabled
        enabled: controlsEnabled

        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
            margins: app.spacing
        }

        //
        // FPS button
        //
        Button {
            enabled: controlsEnabled
            contentItem: Image {
                fillMode: Image.Pad
                source: "qrc:/images/tune.svg"
                verticalAlignment: Image.AlignVCenter
                horizontalAlignment: Image.AlignHCenter
            }

            onClicked: fpsDialog.open()
        }

        //
        // Light button
        //
        Button {
            id: lightButton
            enabled: controlsEnabled
            property bool flashOn: false

            contentItem: Image {
                fillMode: Image.Pad
                verticalAlignment: Image.AlignVCenter
                horizontalAlignment: Image.AlignHCenter
                source: parent.flashOn ? "qrc:/images/flash-on.svg" :
                                         "qrc:/images/flash-off.svg"
            }

            onClicked: {
                flashOn = !flashOn
                QCCTVStation.setFlashlightEnabled (camNumber, flashOn)
            }
        }

        //
        // Sound button
        //
        Button {
            id: soundButton
            enabled: controlsEnabled
            property bool soundOn: true

            contentItem: Image {
                fillMode: Image.Pad
                verticalAlignment: Image.AlignVCenter
                horizontalAlignment: Image.AlignHCenter
                source: parent.soundOn ? "qrc:/images/volume-on.svg" :
                                         "qrc:/images/volume-off.svg"
            }

            onClicked: {
                soundOn = !soundOn
            }
        }

        //
        // Spacer
        //
        Item {
            Layout.fillWidth: true
        }

        //
        // Resolutions ComboBox
        //
        ComboBox {
            id: resolutions
            enabled: controlsEnabled
            model: QCCTVStation.availableResolutions()

            property bool firstIndexChange: true

            contentItem: RowLayout {
                spacing: app.spacing

                Image {
                    fillMode: Image.Pad
                    source: "qrc:/images/aspect-ratio.svg"
                    verticalAlignment: Image.AlignVCenter
                    horizontalAlignment: Image.AlignHCenter
                }

                Label {
                    Layout.fillWidth: true
                    text: qsTr ("Video Resolution")
                }

                Item {
                    Layout.minimumWidth: app.spacing
                }
            }

            onCurrentIndexChanged: {
                if (!firstIndexChange)
                    QCCTVStation.changeResolution (camNumber, currentIndex)
                else
                    firstIndexChange = false
            }
        }
    }
}
