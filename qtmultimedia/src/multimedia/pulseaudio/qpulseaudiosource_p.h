// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of other Qt classes.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#ifndef QAUDIOINPUTPULSE_H
#define QAUDIOINPUTPULSE_H

#include <QtCore/qfile.h>
#include <QtCore/qtimer.h>
#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qelapsedtimer.h>
#include <QtCore/qiodevice.h>

#include "qaudio.h"
#include "qaudiodevice.h"
#include <QtMultimedia/private/qpulsehelpers_p.h>
#include <QtMultimedia/private/qaudiosystem_p.h>
#include <QtMultimedia/private/qaudiostatemachine_p.h>

#include <pulse/pulseaudio.h>

QT_BEGIN_NAMESPACE

class PulseInputPrivate;

class QPulseAudioSource : public QPlatformAudioSource
{
    Q_OBJECT

public:
    QPulseAudioSource(const QByteArray &device, QObject *parent);
    ~QPulseAudioSource() override;

    qint64 read(char *data, qint64 len);

    void start(QIODevice *device) override;
    QIODevice *start() override;
    void stop() override;
    void reset() override;
    void suspend() override;
    void resume() override;
    qsizetype bytesReady() const override;
    void setBufferSize(qsizetype value) override;
    qsizetype bufferSize() const override;
    qint64 processedUSecs() const override;
    QAudio::Error error() const override;
    QAudio::State state() const override;
    void setFormat(const QAudioFormat &format) override;
    QAudioFormat format() const override;

    void setVolume(qreal volume) override;
    qreal volume() const override;

    qint64 m_totalTimeValue;
    QIODevice *m_audioSource;
    QAudioFormat m_format;
    qreal m_volume;

protected:
    void timerEvent(QTimerEvent *event) override;

private slots:
    void userFeed();
    void onPulseContextFailed();

private:
    void applyVolume(const void *src, void *dest, int len);

    bool open();
    void close();

    using PAOperationHandle = QPulseAudioInternal::PAOperationHandle;
    using PAStreamHandle = QPulseAudioInternal::PAStreamHandle;

    bool m_pullMode;
    bool m_opened;
    int m_bufferSize;
    int m_periodSize;
    unsigned int m_periodTime;
    QBasicTimer m_timer;
    qint64 m_elapsedTimeOffset;
    PAStreamHandle m_stream;
    QByteArray m_streamName;
    QByteArray m_device;
    QByteArray m_tempBuffer;
    pa_sample_spec m_spec;

    QAudioStateMachine m_stateMachine;

};

class PulseInputPrivate : public QIODevice
{
    Q_OBJECT
public:
    PulseInputPrivate(QPulseAudioSource *audio);
    ~PulseInputPrivate() override = default;

    qint64 readData(char *data, qint64 len) override;
    qint64 writeData(const char *data, qint64 len) override;

    void trigger();

private:
    QPulseAudioSource *m_audioDevice;
};

QT_END_NAMESPACE

#endif
