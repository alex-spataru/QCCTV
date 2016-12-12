#
# Copyright (c) 2016 Alex Spataru
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#

#-------------------------------------------------------------------------------
# Qt configuration
#-------------------------------------------------------------------------------

TEMPLATE = app
TARGET = QCCTV-Station

QT += svg
QT += core
QT += quick
QT += widgets
QT += quickcontrols2

#-------------------------------------------------------------------------------
# Deploy configuration
#-------------------------------------------------------------------------------

win32* {
    RC_FILE = $$PWD/windows/info.rc
}

macx* {
    ICON = $$PWD/mac/icon.icns
    RC_FILE = $$PWD/mac/icon.icns
    QMAKE_INFO_PLIST = $$PWD/mac/info.plist
}

linux:!android {
    target.path = /usr/bin
    icon.path = /usr/share/pixmaps
    desktop.path = /usr/share/applications
    icon.files += $$PWD/linux/qcctv-station.png
    desktop.files += $$PWD/linux/qcctv-station.desktop

    TARGET = qcctv-station
    INSTALLS += target desktop icon
}

#-------------------------------------------------------------------------------
# Make options
#-------------------------------------------------------------------------------

UI_DIR = uic
MOC_DIR = moc
RCC_DIR = qrc
OBJECTS_DIR = obj

#-------------------------------------------------------------------------------
# Import libraries
#-------------------------------------------------------------------------------

include ($$PWD/../common/qcctv-common.pri)

#-------------------------------------------------------------------------------
# Import QML, resources and source code
#-------------------------------------------------------------------------------

SOURCES += \
    $$PWD/src/main.cpp \
    $$PWD/src/ImageProvider.cpp

RESOURCES += \
    $$PWD/qml/qml.qrc \
    $$PWD/images/images.qrc

OTHER_FILES += \
    $$PWD/qml/*.js \
    $$PWD/qml/*.qml

HEADERS += \
    $$PWD/src/ImageProvider.h
