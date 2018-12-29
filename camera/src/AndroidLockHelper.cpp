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

#if defined __ANDROID__

#include "AndroidLockHelper.h"

AndroidLockHelper::AndroidLockHelper()
{
    QAndroidJniObject activity =
        QAndroidJniObject::callStaticObjectMethod (
            "org/qtproject/qt5/android/QtNative",
            "activity",
            "()Landroid/app/Activity;");

    if (activity.isValid()) {
        QAndroidJniObject serviceName =
            QAndroidJniObject::getStaticObjectField<jstring> (
                "android/content/Context",
                "POWER_SERVICE");

        if (serviceName.isValid()) {
            QAndroidJniObject powerMgr = activity.callObjectMethod (
                                             "getSystemService",
                                             "(Ljava/lang/String;)Ljava/lang/Object;",
                                             serviceName.object<jobject>());

            if (powerMgr.isValid()) {
                jint levelAndFlags = QAndroidJniObject::getStaticField<jint> (
                                         "android/os/PowerManager",
                                         "SCREEN_DIM_WAKE_LOCK");

                QAndroidJniObject tag = QAndroidJniObject::fromString ("My Tag");
                m_wakeLock = powerMgr.callObjectMethod ("newWakeLock",
                                                        "(ILjava/lang/String;)Landroid/os/PowerManager$WakeLock;",
                                                        levelAndFlags, tag.object<jstring>());
            }
        }
    }

    if (m_wakeLock.isValid())
        m_wakeLock.callMethod<void> ("acquire", "()V");
}

AndroidLockHelper::~AndroidLockHelper()
{
    if (m_wakeLock.isValid())
        m_wakeLock.callMethod<void> ("release", "()V");
}

#endif
