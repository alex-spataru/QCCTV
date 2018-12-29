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
import QtQuick.Controls.Material 2.0

Page {
    function focusSearchBox() {
        searchBox.forceActiveFocus()
    }

    Connections {
        target: QCCTVStation
        onCameraCountChanged: listView.loadAllCameras()
    }

    background: Rectangle {
        color: app.backgroundColor

        ColumnLayout {
            spacing: app.spacing
            anchors.fill: parent
            anchors.margins: app.spacing * 2

            //
            // Search box
            //
            RowLayout {
                id: row
                spacing: app.spacing
                Layout.fillWidth: true

                Image {
                    fillMode: Image.Pad
                    sourceSize: Qt.size (48, 48)
                    source: app.getIcon ("search.svg")
                    verticalAlignment: Image.AlignVCenter
                    horizontalAlignment: Image.AlignHCenter
                }

                TextField {
                    id: searchBox
                    Layout.fillWidth: true
                    onTextChanged: listView.search (text)
                    placeholderText: qsTr ("Search for camera feeds") + "..."
                }
            }

            //
            // Search results
            //
            ListView {
                z: 0
                clip: true
                id: listView
                spacing: app.spacing
                Layout.fillWidth: true
                Layout.fillHeight: true

                property variant allowedCameras: []

                //
                // Shows all the cameras in the search results
                //
                function loadAllCameras() {
                    allowedCameras = []
                    model = 0
                    model = QCCTVStation.cameraCount()
                    for (var i = 0; i < model; ++i)
                        allowedCameras.push (i)
                }

                //
                // Only allows the cameras that contain the given \a text
                // to appear in the search result
                //
                function search (text) {
                    allowedCameras = []
                    for (var i = 0; i < model; ++i) {
                        var name = QCCTVStation.cameraName (i).toLowerCase()
                        var filter = text.toLowerCase()
                        if (name.search (filter) > -1 || filter === "")
                            allowedCameras.push (i)
                    }

                    allowedCamerasChanged()
                }

                //
                // Scrollbars
                //
                ScrollBar.vertical: ScrollBar { }

                //
                // Search result item
                //
                delegate: CameraElement {
                    camNumber: index
                    Layout.fillWidth: true

                    Connections {
                        target: listView
                        onAllowedCamerasChanged: {
                            if (listView.allowedCameras.indexOf (index) > -1)
                                opacity = 1
                            else
                                opacity = 0
                        }
                    }
                }
            }
        }
    }
}
