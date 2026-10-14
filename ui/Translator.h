// Translator.h — UI language selection and runtime switching.
//
// English is the source language: every user-facing string in the sources is written
// in English and wrapped in tr(). Chinese (and any future language) ships as a
// compiled .qm catalogue loaded from translations/ next to the executable.
//
// Switching language does not restart the application. install() removes the previous
// catalogue and sends QEvent::LanguageChange to every widget, so each widget can
// re-apply its own strings from its retranslate() method. That is why every UI class in
// this project has one.
#pragma once

#include <QObject>
#include <QString>
#include <QStringList>

QT_BEGIN_NAMESPACE
class QApplication;
class QTranslator;
QT_END_NAMESPACE

namespace spall {

/// The languages the UI can be shown in.
enum class Language {
    English,  ///< the source language; needs no catalogue
    Chinese,  ///< zh_CN, loaded from spall_zh_CN.qm
};

/// Identifies a language for persistence and for the menus.
struct LanguageInfo {
    Language language;
    const char* code;   ///< stable identifier written to QSettings ("en", "zh_CN")
    const char* native; ///< the language's own name, shown in the menu untranslated
};

class Translator : public QObject {
    Q_OBJECT

public:
    explicit Translator(QApplication* app, QObject* parent = nullptr);

    /// Every language the build ships a catalogue for, English first.
    static const QList<LanguageInfo>& available();

    /// Language for a persisted code; falls back to English for unknown codes.
    static Language languageFromCode(const QString& code);
    /// The persisted code for a language.
    static QString codeForLanguage(Language language);

    /// Load the catalogue for `language` and notify every widget. Returns false only
    /// when a non-English catalogue was requested but could not be loaded, in which
    /// case the UI stays in the previous language.
    bool install(Language language);

    Language current() const { return m_current; }

    /// Where catalogue files were looked for, for diagnostics and the About box.
    static QStringList searchPaths();

private:
    QApplication* m_app = nullptr;
    /// SpAll's own catalogue (spall_<locale>.qm).
    QTranslator* m_catalogue = nullptr;
    /// Qt's standard-dialog catalogue (qtbase_<locale>.qm), so QMessageBox and
    /// QFileDialog are localised too.
    QTranslator* m_qtCatalogue = nullptr;
    Language m_current = Language::English;
};

}  // namespace spall
