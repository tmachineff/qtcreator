/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
****************************************************************************/

#include "formwindoweditor.h"
#include "formwindowfile.h"
#include "designerconstants.h"
#include "resourcehandler.h"
#include "designerxmleditorwidget.h"

#include <coreplugin/coreconstants.h>
#include <texteditor/basetextdocument.h>

#include <utils/qtcassert.h>

#if QT_VERSION >= 0x050000
#    include <QDesignerFormWindowInterface>
#    include <QBuffer>
#else
#    include "qt_private/formwindowbase_p.h"
#endif

#include <QDebug>
#include <QFileInfo>
#include <QApplication>

namespace Designer {

struct FormWindowEditorPrivate
{
    Internal::DesignerXmlEditorWidget *m_widget;
};

FormWindowEditor::FormWindowEditor(Internal::DesignerXmlEditorWidget *editor) :
    TextEditor::PlainTextEditor(editor),
    d(new FormWindowEditorPrivate)
{
    d->m_widget = editor;
    setContext(Core::Context(Designer::Constants::K_DESIGNER_XML_EDITOR_ID,
                             Designer::Constants::C_DESIGNER_XML_EDITOR));

    // Revert to saved/load externally modified files.
    connect(d->m_widget->formWindowFile(), SIGNAL(reloadRequested(QString*,QString)),
            this, SLOT(slotOpen(QString*,QString)), Qt::DirectConnection);
}

FormWindowEditor::~FormWindowEditor()
{
    delete d;
}

bool FormWindowEditor::createNew(const QString &contents)
{
    if (Designer::Constants::Internal::debug)
        qDebug() << "FormWindowEditor::createNew" << contents.size();

    syncXmlEditor(QString());

    QDesignerFormWindowInterface *form = d->m_widget->formWindowFile()->formWindow();
    QTC_ASSERT(form, return false);

    if (contents.isEmpty())
        return false;

    // If we have an override cursor, reset it over Designer loading,
    // should it pop up messages about missing resources or such.
    const bool hasOverrideCursor = QApplication::overrideCursor();
    QCursor overrideCursor;
    if (hasOverrideCursor) {
        overrideCursor = QCursor(*QApplication::overrideCursor());
        QApplication::restoreOverrideCursor();
    }

#if QT_VERSION >= 0x050000
    const bool success = form->setContents(contents);
#else
    form->setContents(contents);
    const bool success = form->mainContainer() != 0;
#endif

    if (hasOverrideCursor)
        QApplication::setOverrideCursor(overrideCursor);

    if (!success)
        return false;

    syncXmlEditor(contents);
    d->m_widget->formWindowFile()->setFilePath(QString());
    d->m_widget->formWindowFile()->setShouldAutoSave(false);
    return true;
}

void FormWindowEditor::slotOpen(QString *errorString, const QString &fileName)
{
    open(errorString, fileName, fileName);
}

bool FormWindowEditor::open(QString *errorString, const QString &fileName, const QString &realFileName)
{
    if (Designer::Constants::Internal::debug)
        qDebug() << "FormWindowEditor::open" << fileName;

    QDesignerFormWindowInterface *form = d->m_widget->formWindowFile()->formWindow();
    QTC_ASSERT(form, return false);

    if (fileName.isEmpty())
        return true;

    const QFileInfo fi(fileName);
    const QString absfileName = fi.absoluteFilePath();

    QString contents;
    if (d->m_widget->formWindowFile()->read(absfileName, &contents, errorString) != Utils::TextFileFormat::ReadSuccess)
        return false;

    form->setFileName(absfileName);
#if QT_VERSION >= 0x050000
    const QByteArray contentsBA = contents.toUtf8();
    QBuffer str;
    str.setData(contentsBA);
    str.open(QIODevice::ReadOnly);
    if (!form->setContents(&str, errorString))
        return false;
#else
    form->setContents(contents);
    if (!form->mainContainer())
        return false;
#endif
    form->setDirty(fileName != realFileName);
    syncXmlEditor(contents);

    d->m_widget->formWindowFile()->setFilePath(absfileName);
    d->m_widget->formWindowFile()->setShouldAutoSave(false);

    if (Internal::ResourceHandler *rh = form->findChild<Designer::Internal::ResourceHandler*>())
        rh->updateResources();

    return true;
}

void FormWindowEditor::syncXmlEditor()
{
    if (Designer::Constants::Internal::debug)
        qDebug() << "FormWindowEditor::syncXmlEditor" << d->m_widget->formWindowFile()->filePath();
    syncXmlEditor(contents());
}

void FormWindowEditor::syncXmlEditor(const QString &contents)
{
    editorWidget()->document()->setPlainText(contents);
}

Core::Id FormWindowEditor::id() const
{
    return Core::Id(Designer::Constants::K_DESIGNER_XML_EDITOR_ID);
}

QWidget *FormWindowEditor::toolBar()
{
    return 0;
}

QString FormWindowEditor::contents() const
{
#if QT_VERSION >= 0x050000    // TODO: No warnings about spacers here
    const QDesignerFormWindowInterface *fw = d->m_widget->formWindowFile()->formWindow();
    QTC_ASSERT(fw, return QString());
    return fw->contents();
#else
    // No warnings about spacers here
    const qdesigner_internal::FormWindowBase *fw =
            qobject_cast<const qdesigner_internal::FormWindowBase *>(d->m_widget->formWindowFile()->formWindow());
    QTC_ASSERT(fw, return QString());
    return fw->fileContents();
#endif
}

bool FormWindowEditor::isDesignModePreferred() const
{
    return true;
}

} // namespace Designer

