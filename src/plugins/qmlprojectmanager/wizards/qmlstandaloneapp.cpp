/**************************************************************************
**
** This file is part of Qt Creator
**
** Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
**
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** Commercial Usage
**
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
** GNU Lesser General Public License Usage
**
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at http://qt.nokia.com/contact.
**
**************************************************************************/

#include "qmlstandaloneapp.h"

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QRegExp>
#include <QtCore/QTextStream>

namespace QmlProjectManager {
namespace Internal {

QmlStandaloneApp::QmlStandaloneApp()
    : m_loadDummyData(false)
    , m_orientation(Auto)
    , m_networkEnabled(false)
{
}

QString QmlStandaloneApp::symbianUidForPath(const QString &path)
{
    quint32 hash = 5381;
    for (int i = 0; i < path.size(); ++i) {
        const char c = path.at(i).toAscii();
        hash ^= c + ((c - i) << i % 20) + ((c + i) << (i + 5) % 20) + ((c - 2 * i) << (i + 10) % 20) + ((c + 2 * i) << (i + 15) % 20);
    }
    return QString::fromLatin1("0xE")
            + QString::fromLatin1("%1").arg(hash, 7, 16, QLatin1Char('0')).right(7);
}

void QmlStandaloneApp::setOrientation(Orientation orientation)
{
    m_orientation = orientation;
}

QmlStandaloneApp::Orientation QmlStandaloneApp::orientation() const
{
    return m_orientation;
}

void QmlStandaloneApp::setProjectName(const QString &name)
{
    m_projectName = name;
}

QString QmlStandaloneApp::projectName() const
{
    return m_projectName;
}

void QmlStandaloneApp::setProjectPath(const QString &path)
{
    m_projectPath = path;
    if (!(m_projectPath.endsWith(QLatin1Char('\\'))
        || m_projectPath.endsWith(QLatin1Char('/'))))
        m_projectPath.append(QDir::separator());
}

void QmlStandaloneApp::setSymbianSvgIcon(const QString &icon)
{
    m_symbianSvgIcon = icon;
}

QString QmlStandaloneApp::symbianSvgIcon() const
{
    return path(SymbianSvgIcon, Source);
}

void QmlStandaloneApp::setSymbianTargetUid(const QString &uid)
{
    m_symbianTargetUid = uid;
}

QString QmlStandaloneApp::symbianTargetUid() const
{
    return !m_symbianTargetUid.isEmpty() ? m_symbianTargetUid
        : symbianUidForPath(m_projectPath + m_projectName);
}

void QmlStandaloneApp::setLoadDummyData(bool loadIt)
{
    m_loadDummyData = loadIt;
}

bool QmlStandaloneApp::loadDummyData() const
{
    return m_loadDummyData;
}

void QmlStandaloneApp::setNetworkEnabled(bool enabled)
{
    m_networkEnabled = enabled;
}

bool QmlStandaloneApp::networkEnabled() const
{
    return m_networkEnabled;
}

QString QmlStandaloneApp::path(Path path, Location location) const
{
    const QString sourceRoot = QLatin1String(":/qmlproject/wizards/templates/");
    const QString cppSourceSubDir = QLatin1String("cpp/");
    const QString qmlSourceSubDir = QLatin1String("qml/");
    const QString cppTargetSubDir = cppSourceSubDir;
    const QString qmlTargetSubDir = qmlSourceSubDir;
    const QString qmlExtension = QLatin1String(".qml");
    const QString mainCpp = QLatin1String("main.cpp");
    const QString appViewCpp = QLatin1String("qmlapplicationview.cpp");
    const QString appViewH = QLatin1String("qmlapplicationview.h");
    const QString symbianIcon = QLatin1String("symbianicon.svg");
    const char* const errorMessage = "QmlStandaloneApp::path() needs more work";

    switch (location) {
        case Source: {
            switch (path) {
                case MainQml:           return sourceRoot + qmlSourceSubDir + QLatin1String("app.qml");
                case AppProfile:        return sourceRoot + QLatin1String("app.pro");
                case MainCpp:           return sourceRoot + cppSourceSubDir + mainCpp;
                case AppViewerCpp:      return sourceRoot + cppSourceSubDir + appViewCpp;
                case AppViewerH:        return sourceRoot + cppSourceSubDir + appViewH;
                case SymbianSvgIcon:    return !m_symbianSvgIcon.isEmpty() ? m_symbianSvgIcon
                                                    :sourceRoot + cppSourceSubDir + symbianIcon;
                default:                qFatal(errorMessage);
            }
        }
        case Target: {
            const QString pathBase = m_projectPath + m_projectName + QDir::separator();
            switch (path) {
                case MainQml:           return pathBase + qmlTargetSubDir + m_projectName + qmlExtension;
                case AppProfile:        return pathBase + m_projectName + QLatin1String(".pro");
                case MainCpp:           return pathBase + cppTargetSubDir + mainCpp;
                case AppViewerCpp:      return pathBase + cppTargetSubDir + appViewCpp;
                case AppViewerH:        return pathBase + cppTargetSubDir + appViewH;
                case SymbianSvgIcon:    return pathBase + cppTargetSubDir + symbianIcon;
                default:                qFatal(errorMessage);
            }
        }
        case AppProfileRelative: {
            switch (path) {
                case MainQml:           return qmlTargetSubDir + m_projectName + qmlExtension;
                case MainCpp:           return cppTargetSubDir + mainCpp;
                case AppViewerCpp:      return cppTargetSubDir + appViewCpp;
                case AppViewerH:        return cppTargetSubDir + appViewH;
                case SymbianSvgIcon:    return cppTargetSubDir + symbianIcon;
                case QmlDir:            return QString(qmlTargetSubDir).remove(qmlTargetSubDir.length() - 1, 1);
                default:                qFatal(errorMessage);
            }
        }
        default: { /* case MainCppRelative: */
            switch (path) {
                case MainQml:           return qmlTargetSubDir + m_projectName + qmlExtension;
                default:                qFatal(errorMessage);
            }
        }
    }
    return QString();
}


static QString insertParameter(const QString &line, const QString &parameter)
{
    return QString(line).replace(QRegExp(QLatin1String("\\([^()]+\\)")),
                                 QLatin1Char('(') + parameter + QLatin1Char(')'));
}

QByteArray QmlStandaloneApp::generateMainCpp(const QString *errorMessage) const
{
    Q_UNUSED(errorMessage)

    QFile sourceFile(path(MainCpp, Source));
    sourceFile.open(QIODevice::ReadOnly);
    Q_ASSERT(sourceFile.isOpen());
    QTextStream in(&sourceFile);

    QByteArray mainCppContent;
    QTextStream out(&mainCppContent, QIODevice::WriteOnly);

    QString line;
    do {
        line = in.readLine();
        if (line.contains(QLatin1String("// MAINQML"))) {
            line = insertParameter(line, QLatin1Char('"')
                                   + path(MainQml, MainCppRelative) + QLatin1Char('"'));
        } else if (line.contains(QLatin1String("// IMPORTPATHSLIST"))) {
            continue;
        } else if (line.contains(QLatin1String("// SETIMPORTPATHLIST"))) {
            continue;
        } else if (line.contains(QLatin1String("// ORIENTATION"))) {
            if (m_orientation == Auto)
                continue;
            else
                line = insertParameter(line, QLatin1String("QmlApplicationView::")
                                       + QLatin1String(m_orientation == LockLandscape ?
                                                       "LockLandscape" : "LockPortrait"));
        } else if (line.contains(QLatin1String("// LOADDUMMYDATA"))) {
            continue;
        }
        const int commentIndex = line.indexOf(QLatin1String(" //"));
        if (commentIndex != -1)
            line.truncate(commentIndex);
        out << line << endl;
    } while (!line.isNull());

    return mainCppContent;
}

QByteArray QmlStandaloneApp::generateProFile(const QString *errorMessage) const
{
    Q_UNUSED(errorMessage)

    QFile proFile(path(AppProfile, Source));
    proFile.open(QIODevice::ReadOnly);
    Q_ASSERT(proFile.isOpen());
    QTextStream in(&proFile);

    QByteArray proFileContent;
    QTextStream out(&proFileContent, QIODevice::WriteOnly);

    QString line;
    QString valueOnNextLine;
    bool uncommentNextLine = false;
    do {
        line = in.readLine();

        if (line.contains(QLatin1String("# MAINQML"))) {
            valueOnNextLine = path(MainQml, AppProfileRelative);
        } else if (line.contains(QLatin1String("# TARGETUID3"))) {
            valueOnNextLine = symbianTargetUid();
        } else if (line.contains(QLatin1String("# DEPLOYMENTFOLDERS"))) {
            valueOnNextLine = path(QmlDir, AppProfileRelative);
        } else if (line.contains(QLatin1String("# ORIENTATIONLOCK")) && m_orientation == QmlStandaloneApp::Auto) {
            uncommentNextLine = true;
        } else if (line.contains(QLatin1String("# NETWORKACCESS")) && !m_networkEnabled) {
            uncommentNextLine = true;
        }

        // Remove all marker comments
        if (line.trimmed().startsWith(QLatin1Char('#'))
            && line.trimmed().endsWith(QLatin1Char('#')))
            continue;

        if (!valueOnNextLine.isEmpty()) {
            out << line.left(line.indexOf(QLatin1Char('=')) + 2)
                    << QDir::fromNativeSeparators(valueOnNextLine) << endl;
            valueOnNextLine.clear();
            continue;
        }

        if (uncommentNextLine) {
            out << QLatin1String("#") << line << endl;
            uncommentNextLine = false;
            continue;
        }
        out << line << endl;
    } while (!line.isNull());

    return proFileContent;
}

#ifndef CREATORLESSTEST
static Core::GeneratedFile generateFileCopy(const QString &source,
                                            const QString &target,
                                            bool openEditor = false)
{
    QFile sourceFile(source);
    sourceFile.open(QIODevice::ReadOnly);
    Q_ASSERT(sourceFile.isOpen());
    Core::GeneratedFile generatedFile(target);
    generatedFile.setBinary(true);
    generatedFile.setBinaryContents(sourceFile.readAll());
    if (openEditor)
        generatedFile.setAttributes(Core::GeneratedFile::OpenEditorAttribute);
    return generatedFile;
}

Core::GeneratedFiles QmlStandaloneApp::generateFiles(QString *errorMessage) const
{
    Core::GeneratedFiles files;

    Core::GeneratedFile generatedProFile(path(AppProfile, Target));
    generatedProFile.setContents(generateProFile(errorMessage));
    generatedProFile.setAttributes(Core::GeneratedFile::OpenProjectAttribute);
    files.append(generatedProFile);

    files.append(generateFileCopy(path(MainQml, Source), path(MainQml, Target), true));

    Core::GeneratedFile generatedMainCppFile(path(MainCpp, Target));
    generatedMainCppFile.setContents(generateMainCpp(errorMessage));
    files.append(generatedMainCppFile);

    files.append(generateFileCopy(path(AppViewerCpp, Source), path(AppViewerCpp, Target)));
    files.append(generateFileCopy(path(AppViewerH, Source), path(AppViewerH, Target)));
    files.append(generateFileCopy(path(SymbianSvgIcon, Source), path(SymbianSvgIcon, Target)));

    return files;
}
#endif // CREATORLESSTEST

} // namespace Internal
} // namespace QmlProjectManager
