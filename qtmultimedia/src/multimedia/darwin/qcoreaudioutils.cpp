// Copyright (C) 2016 The Qt Company Ltd and/or its subsidiary(-ies).
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qcoreaudioutils_p.h"
#include <qdebug.h>

QT_BEGIN_NAMESPACE

namespace QCoreAudioUtils {

QAudioFormat toQAudioFormat(AudioStreamBasicDescription const& sf)
{
    QAudioFormat    audioFormat;
    // all Darwin HW is little endian, we ignore those formats
    if ((sf.mFormatFlags & kAudioFormatFlagIsBigEndian) != 0 && QSysInfo::ByteOrder != QSysInfo::LittleEndian)
        return audioFormat;

    // filter out the formats we're interested in
    QAudioFormat::SampleFormat format = QAudioFormat::Unknown;
    switch (sf.mBitsPerChannel) {
    case 8:
        if ((sf.mFormatFlags & kAudioFormatFlagIsSignedInteger) == 0)
            format = QAudioFormat::UInt8;
        break;
    case 16:
        if ((sf.mFormatFlags & kAudioFormatFlagIsSignedInteger) != 0)
            format = QAudioFormat::Int16;
        break;
    case 32:
        if ((sf.mFormatFlags & kAudioFormatFlagIsSignedInteger) != 0)
            format = QAudioFormat::Int32;
        else if ((sf.mFormatFlags & kAudioFormatFlagIsFloat) != 0)
            format = QAudioFormat::Float;
        break;
    default:
        break;
    }

    audioFormat.setSampleFormat(format);
    audioFormat.setSampleRate(sf.mSampleRate);
    audioFormat.setChannelCount(sf.mChannelsPerFrame);

    return audioFormat;
}

AudioStreamBasicDescription toAudioStreamBasicDescription(QAudioFormat const& audioFormat)
{
    AudioStreamBasicDescription sf;

    sf.mFormatFlags         = kAudioFormatFlagIsPacked;
    sf.mSampleRate          = audioFormat.sampleRate();
    sf.mFramesPerPacket     = 1;
    sf.mChannelsPerFrame    = audioFormat.channelCount();
    sf.mBitsPerChannel      = audioFormat.bytesPerSample() * 8;
    sf.mBytesPerFrame       = audioFormat.bytesPerFrame();
    sf.mBytesPerPacket      = sf.mFramesPerPacket * sf.mBytesPerFrame;
    sf.mFormatID            = kAudioFormatLinearPCM;

    switch (audioFormat.sampleFormat()) {
    case QAudioFormat::Int16:
    case QAudioFormat::Int32:
        sf.mFormatFlags |= kAudioFormatFlagIsSignedInteger;
        break;
    case QAudioFormat::Float:
        sf.mFormatFlags |= kAudioFormatFlagIsFloat;
        break;
    case QAudioFormat::UInt8:
        /* default */
    case QAudioFormat::Unknown:
    case QAudioFormat::NSampleFormats:
        break;
    }

    return sf;
}


static constexpr struct {
    QAudioFormat::AudioChannelPosition pos;
    AudioChannelLabel label;
} channelMap[] =  {
        { QAudioFormat::FrontLeft, kAudioChannelLabel_Left },
        { QAudioFormat::FrontRight, kAudioChannelLabel_Right },
        { QAudioFormat::FrontCenter, kAudioChannelLabel_Center },
        { QAudioFormat::LFE, kAudioChannelLabel_LFEScreen },
        { QAudioFormat::BackLeft, kAudioChannelLabel_LeftSurround },
        { QAudioFormat::BackRight, kAudioChannelLabel_RightSurround },
        { QAudioFormat::FrontLeftOfCenter, kAudioChannelLabel_LeftCenter },
        { QAudioFormat::FrontRightOfCenter, kAudioChannelLabel_RightCenter },
        { QAudioFormat::BackCenter, kAudioChannelLabel_CenterSurround },
        { QAudioFormat::LFE2, kAudioChannelLabel_LFE2 },
        { QAudioFormat::SideLeft, kAudioChannelLabel_LeftSurroundDirect }, // ???
        { QAudioFormat::SideRight, kAudioChannelLabel_RightSurroundDirect }, // ???
        { QAudioFormat::TopFrontLeft, kAudioChannelLabel_VerticalHeightLeft },
        { QAudioFormat::TopFrontRight, kAudioChannelLabel_VerticalHeightRight },
        { QAudioFormat::TopFrontCenter, kAudioChannelLabel_VerticalHeightCenter },
        { QAudioFormat::TopCenter, kAudioChannelLabel_CenterTopMiddle },
        { QAudioFormat::TopBackLeft, kAudioChannelLabel_TopBackLeft },
        { QAudioFormat::TopBackRight, kAudioChannelLabel_TopBackRight },
        { QAudioFormat::TopSideLeft, kAudioChannelLabel_LeftTopMiddle },
        { QAudioFormat::TopSideRight, kAudioChannelLabel_RightTopMiddle },
        { QAudioFormat::TopBackCenter, kAudioChannelLabel_TopBackCenter },
};

std::unique_ptr<AudioChannelLayout, QFreeDeleter>
toAudioChannelLayout(const QAudioFormat &format, UInt32 *size)
{
    auto channelConfig = format.channelConfig();
    if (channelConfig == QAudioFormat::ChannelConfigUnknown)
        channelConfig = QAudioFormat::defaultChannelConfigForChannelCount(format.channelCount());

    *size = sizeof(AudioChannelLayout) + int(QAudioFormat::NChannelPositions)*sizeof(AudioChannelDescription);
    auto *layout = static_cast<AudioChannelLayout *>(malloc(*size));
    memset(layout, 0, *size);
    layout->mChannelLayoutTag = kAudioChannelLayoutTag_UseChannelDescriptions;

    for (const auto &m : channelMap) {
        if (channelConfig & QAudioFormat::channelConfig(m.pos))
            layout->mChannelDescriptions[layout->mNumberChannelDescriptions++].mChannelLabel = m.label;
    }

    if (channelConfig & QAudioFormat::channelConfig(QAudioFormat::BottomFrontCenter)) {
        auto &desc = layout->mChannelDescriptions[layout->mNumberChannelDescriptions++];
        desc.mChannelLabel = kAudioChannelLabel_UseCoordinates;
        desc.mChannelFlags = kAudioChannelFlags_SphericalCoordinates;
        desc.mCoordinates[kAudioChannelCoordinates_Azimuth] = 0.f;
        desc.mCoordinates[kAudioChannelCoordinates_Elevation] = -20.;
        desc.mCoordinates[kAudioChannelCoordinates_Distance] = 1.f;
    }
    if (channelConfig & QAudioFormat::channelConfig(QAudioFormat::BottomFrontLeft)) {
        auto &desc = layout->mChannelDescriptions[layout->mNumberChannelDescriptions++];
        desc.mChannelLabel = kAudioChannelLabel_UseCoordinates;
        desc.mChannelFlags = kAudioChannelFlags_SphericalCoordinates;
        desc.mCoordinates[kAudioChannelCoordinates_Azimuth] = -45.f;
        desc.mCoordinates[kAudioChannelCoordinates_Elevation] = -20.;
        desc.mCoordinates[kAudioChannelCoordinates_Distance] = 1.f;
    }
    if (channelConfig & QAudioFormat::channelConfig(QAudioFormat::BottomFrontRight)) {
        auto &desc = layout->mChannelDescriptions[layout->mNumberChannelDescriptions++];
        desc.mChannelLabel = kAudioChannelLabel_UseCoordinates;
        desc.mChannelFlags = kAudioChannelFlags_SphericalCoordinates;
        desc.mCoordinates[kAudioChannelCoordinates_Azimuth] = 45.f;
        desc.mCoordinates[kAudioChannelCoordinates_Elevation] = -20.;
        desc.mCoordinates[kAudioChannelCoordinates_Distance] = 1.f;
    }

    return std::unique_ptr<AudioChannelLayout, QFreeDeleter>(layout);
}

static constexpr struct {
    AudioChannelLayoutTag tag;
    QAudioFormat::ChannelConfig channelConfig;
} layoutTagMap[] = {
    { kAudioChannelLayoutTag_Mono, QAudioFormat::ChannelConfigMono },
    { kAudioChannelLayoutTag_Stereo, QAudioFormat::ChannelConfigStereo },
    { kAudioChannelLayoutTag_StereoHeadphones, QAudioFormat::ChannelConfigStereo },
    { kAudioChannelLayoutTag_MPEG_1_0, QAudioFormat::ChannelConfigMono },
    { kAudioChannelLayoutTag_MPEG_2_0, QAudioFormat::ChannelConfigStereo },
    { kAudioChannelLayoutTag_MPEG_3_0_A, QAudioFormat::channelConfig(QAudioFormat::FrontLeft,
                                                                     QAudioFormat::FrontRight,
                                                                     QAudioFormat::FrontCenter) },
    { kAudioChannelLayoutTag_MPEG_4_0_A, QAudioFormat::channelConfig(QAudioFormat::FrontLeft,
                                                                     QAudioFormat::FrontRight,
                                                                     QAudioFormat::FrontCenter,
                                                                     QAudioFormat::BackCenter) },
    { kAudioChannelLayoutTag_MPEG_5_0_A, QAudioFormat::ChannelConfigSurround5Dot0 },
    { kAudioChannelLayoutTag_MPEG_5_1_A, QAudioFormat::ChannelConfigSurround5Dot1 },
    { kAudioChannelLayoutTag_MPEG_6_1_A, QAudioFormat::channelConfig(QAudioFormat::FrontLeft,
                                                                     QAudioFormat::FrontRight,
                                                                     QAudioFormat::FrontCenter,
                                                                     QAudioFormat::LFE,
                                                                     QAudioFormat::BackLeft,
                                                                     QAudioFormat::BackRight,
                                                                     QAudioFormat::BackCenter) },
    { kAudioChannelLayoutTag_MPEG_7_1_A, QAudioFormat::ChannelConfigSurround7Dot1 },
    { kAudioChannelLayoutTag_SMPTE_DTV, QAudioFormat::channelConfig(QAudioFormat::FrontLeft,
                                                                    QAudioFormat::FrontRight,
                                                                    QAudioFormat::FrontCenter,
                                                                    QAudioFormat::LFE,
                                                                    QAudioFormat::BackLeft,
                                                                    QAudioFormat::BackRight,
                                                                    QAudioFormat::TopFrontLeft,
                                                                    QAudioFormat::TopFrontRight) },

    { kAudioChannelLayoutTag_ITU_2_1, QAudioFormat::ChannelConfig2Dot1 },
    { kAudioChannelLayoutTag_ITU_2_2, QAudioFormat::channelConfig(QAudioFormat::FrontLeft,
                                                                  QAudioFormat::FrontRight,
                                                                  QAudioFormat::BackLeft,
                                                                  QAudioFormat::BackRight) }
};


QAudioFormat::ChannelConfig fromAudioChannelLayout(const AudioChannelLayout *layout)
{
    for (const auto &m : layoutTagMap) {
        if (m.tag == layout->mChannelLayoutTag)
            return m.channelConfig;
    }

    quint32 channels = 0;
    if (layout->mChannelLayoutTag == kAudioChannelLayoutTag_UseChannelDescriptions) {
        // special case 1 and 2 channel configs, as they are often reported without proper descriptions
        if (layout->mNumberChannelDescriptions == 1
            && (layout->mChannelDescriptions[0].mChannelLabel == kAudioChannelLabel_Unknown
                || layout->mChannelDescriptions[0].mChannelLabel == kAudioChannelLabel_Mono))
            return QAudioFormat::ChannelConfigMono;
        if (layout->mNumberChannelDescriptions == 2 &&
            layout->mChannelDescriptions[0].mChannelLabel == kAudioChannelLabel_Unknown &&
            layout->mChannelDescriptions[1].mChannelLabel == kAudioChannelLabel_Unknown)
            return QAudioFormat::ChannelConfigStereo;

        for (uint i = 0; i < layout->mNumberChannelDescriptions; ++i) {
            const auto channelLabel = layout->mChannelDescriptions[i].mChannelLabel;
            if (channelLabel == kAudioChannelLabel_Unknown) {
                // Any number of unknown channel labels occurs for loopback audio devices.
                // E.g. the case is reproduced with installed software Soundflower.
                continue;
            }

            const auto found = std::find_if(channelMap, std::end(channelMap),
                                            [channelLabel](const auto &labelWithPos) {
                                                return labelWithPos.label == channelLabel;
                                            });

            if (found == std::end(channelMap))
                qWarning() << "audio device has unrecognized channel, index:" << i
                           << "label:" << channelLabel;
            else
                channels |= QAudioFormat::channelConfig(found->pos);
        }
    } else {
        qWarning() << "Channel layout uses unimplemented format, channelLayoutTag:"
                   << layout->mChannelLayoutTag;
    }
    return QAudioFormat::ChannelConfig(channels);
}

} // namespace QCoreAudioUtils

QT_END_NAMESPACE
