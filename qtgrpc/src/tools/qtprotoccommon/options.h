// Copyright (C) 2022 The Qt Company Ltd.
// Copyright (C) 2019 Alexey Edelev <semlanik@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef GENERATOROPTIONS_H
#define GENERATOROPTIONS_H

#include <string>

namespace qtprotoccommon {

/*!
 \ingroup generator
 \private
 \brief The GeneratorOptions class
 */
class Options
{
    Options();

    static Options &mutableInstance();
public:
    enum GeneratorType {
        QtProtobufGen = 0,
        QtGrpcGen
    };
    ~Options();
    Options(const Options &) = delete;
    Options &operator=(const Options &) = delete;
    Options(Options &&) = delete;
    Options &operator=(Options &&) = delete;

    static const Options &instance();
    static void setFromString(const std::string &options, GeneratorType type = QtProtobufGen);

    bool hasQml() const { return m_qml; }
    bool generateComments() const { return m_generateComments; }
    bool isFolder() const { return m_isFolder; }
    bool generateFieldEnum() const { return m_generateFieldEnum; }
    const std::string &extraNamespace() const { return m_extraNamespace; }
    const std::string &exportMacro() const { return m_exportMacro; }
    const std::string &exportMacroFilename() const { return m_exportMacroFilename; }
    bool generateMacroExportFile() const { return m_generateMacroExportFile; }

private:
    bool m_generateComments;
    bool m_isFolder;
    bool m_generateFieldEnum;
    std::string m_extraNamespace;
    std::string m_exportMacro;
    std::string m_exportMacroFilename;
    bool m_generateMacroExportFile;
    bool m_qml;
};

}
#endif // GENERATOROPTIONS_H
