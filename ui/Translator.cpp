// Translator.cpp — catalogue loading and language switching.
#include "Translator.h"

#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QEvent>
#include <QFileInfo>
#include <QLibraryInfo>
#include <QLocale>
#include <QTranslator>
#include <QWidget>

#include "StartupTrace.h"

namespace spall {

namespace {

/// Catalogue base name without the locale suffix, e.g. "spall_zh_CN.qm".
QString catalogueName(Language language) {
    return QStringLiteral("spall_%1").arg(Translator::codeForLanguage(language));
}

}  // namespace

Translator::Translator(QApplication* app, QObject* parent)
    : QObject(parent),
      m_app(app),
      m_catalogue(new QTranslator(this)),
      m_qtCatalogue(new QTranslator(this)) {}

const QList<LanguageInfo>& Translator::available() {
    // English first: it is the source language and the default, so it is also the
    // entry a user looks for first.
    static const QList<LanguageInfo> languages = {
        {Language::English, "en", "English"},
        {Language::Chinese, "zh_CN", "简体中文"},
    };
    return languages;
}

Language Translator::languageFromCode(const QString& code) {
    for (const LanguageInfo& info : available()) {
        if (code == QLatin1String(info.code)) {
            return info.language;
        }
    }
    return Language::English;
}

QString Translator::codeForLanguage(Language language) {
    for (const LanguageInfo& info : available()) {
        if (info.language == language) {
            return QString::fromLatin1(info.code);
        }
    }
    return QStringLiteral("en");
}

QStringList Translator::searchPaths() {
    // The build copies catalogues next to the executable. The source-tree locations are
    // included so a developer can run straight out of the build tree, and Qt's own
    // translation directory is searched so the standard dialogs (QFileDialog,
    // QMessageBox buttons, ...) are localised too.
    QStringList paths;
    const QString appDir = QCoreApplication::applicationDirPath();
    paths << appDir + QStringLiteral("/translations")
          << appDir
          << appDir + QStringLiteral("/../translations")
          << appDir + QStringLiteral("/../../translations");
    const QString qtTranslations = QLibraryInfo::path(QLibraryInfo::TranslationsPath);
    if (!qtTranslations.isEmpty()) {
        paths << qtTranslations;
    }
    return paths;
}

bool Translator::install(Language language) {
    if (m_app == nullptr) {
        return false;
    }
    startupTrace("translator: install enter");

    // Drop the previous catalogues first. Retranslation below then sees only the new
    // ones, so a switch never leaves strings from two languages on screen.
    if (!m_catalogue->isEmpty()) {
        m_app->removeTranslator(m_catalogue);
    }
    if (!m_qtCatalogue->isEmpty()) {
        m_app->removeTranslator(m_qtCatalogue);
    }
    startupTrace("translator: old catalogues removed");

    const QString code = codeForLanguage(language);
    const QString name = catalogueName(language);
    bool loaded = false;
    for (const QString& dir : searchPaths()) {
        if (m_catalogue->load(name, dir)) {
            m_app->installTranslator(m_catalogue);
            loaded = true;
            break;
        }
    }
    startupTrace("translator: catalogue loaded");

    if (!loaded && language != Language::English) {
        return false;
    }

    // Qt's own strings — QMessageBox buttons, QFileDialog, the line-edit context menu —
    // live in qtbase_<locale>.qm inside Qt's translations directory. Without it the
    // standard dialogs would stay English while the rest of the UI switched.
    // A missing file is not an error: the source language simply shows through.
    if (language != Language::English) {
        for (const QString& dir : searchPaths()) {
            if (m_qtCatalogue->load(QStringLiteral("qtbase_%1").arg(code), dir)) {
                m_app->installTranslator(m_qtCatalogue);
                break;
            }
        }
    }
    startupTrace("translator: qt catalogue handled");

    m_current = language;

    // Tell every widget to re-read its strings. Qt delivers this to all top-level
    // widgets and their children, so each retranslate() runs once.
    const QList<QWidget*> widgets = m_app->allWidgets();
    startupTrace("translator: widget list obtained");
    QEvent languageChange(QEvent::LanguageChange);
    for (QWidget* widget : widgets) {
        QCoreApplication::sendEvent(widget, &languageChange);
    }
    startupTrace("translator: install done");
    return true;
}

}  // namespace spall
