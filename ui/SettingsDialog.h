// SettingsDialog.h — editor / compile / appearance settings.
#pragma once

#include <QDialog>

#include "CompileOptions.h"

QT_BEGIN_NAMESPACE
class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QPlainTextEdit;
class QSpinBox;
class QTabWidget;
QT_END_NAMESPACE

#include "Translator.h"

namespace spall {

/// A snapshot of every persisted setting.
struct AppSettings {
    // --- compilation --------------------------------------------------------
    int glslVersion = 450;
    VulkanVersion vulkanVersion = VulkanVersion::Vulkan13;
    TargetEnv targetEnv = TargetEnv::Vulkan13;
    SpirvVersion spirvVersion = SpirvVersion::Spv16;
    bool autoMapLocations = true;
    bool autoMapBindings = true;
    bool enableIncludes = true;
    bool validate = true;
    bool enablePreamble = true;
    QStringList includePaths;
    GlslangResourceLimits limits;

    // --- build / disassembly defaults --------------------------------------
    OptimizationLevel optimizationLevel = OptimizationLevel::None;
    /// Bitwise OR of DisasmFlag values; persists the assembly panel checkboxes.
    unsigned disasmFlags = DisasmIndent | DisasmFriendlyNames | DisasmComment;

    // --- editor -------------------------------------------------------------
    QString fontFamily;
    int fontSize = 10;
    int tabWidth = 4;
    bool softTabs = true;
    bool completionEnabled = true;
    bool syntaxHintsEnabled = true;

    // --- appearance ---------------------------------------------------------
    QString theme = QStringLiteral("system");  // system | light | dark
    bool hideUnusedReflection = false;

    // --- interface ----------------------------------------------------------
    /// UI language code, from Translator::codeForLanguage(). English is the source
    /// language and the default; anything else loads a catalogue from translations/.
    QString language = QStringLiteral("en");

    /// Load from QSettings.
    static AppSettings load();
    /// Persist to QSettings.
    void save() const;

    /// Turn the editor-independent half into CompileOptions defaults.
    CompileOptions toCompileOptions(ShaderStage stage) const;
    /// Apply the compile-related fields from a CompileOptions.
    void applyFrom(const CompileOptions& options);
};

/// Modal settings editor.
class SettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit SettingsDialog(const AppSettings& settings, QWidget* parent = nullptr);

    /// The edited values; valid after exec() returns Accepted.
    const AppSettings& settings() const { return m_settings; }

private slots:
    void onAccept();

private:
    QWidget* buildCompilePage();
    QWidget* buildEditorPage();
    QWidget* buildAppearancePage();
    QWidget* buildInterfacePage();

    AppSettings m_settings;
    QTabWidget* m_tabs = nullptr;

    // compile page
    QSpinBox* m_glslVersion = nullptr;
    QComboBox* m_vulkanVersion = nullptr;
    QComboBox* m_targetEnv = nullptr;
    QComboBox* m_spirvVersion = nullptr;
    QCheckBox* m_autoMapLocations = nullptr;
    QCheckBox* m_autoMapBindings = nullptr;
    QCheckBox* m_enableIncludes = nullptr;
    QCheckBox* m_validate = nullptr;
    QCheckBox* m_enablePreamble = nullptr;
    QPlainTextEdit* m_includePaths = nullptr;

    // editor page
    QLineEdit* m_fontFamily = nullptr;
    QSpinBox* m_fontSize = nullptr;
    QSpinBox* m_tabWidth = nullptr;
    QCheckBox* m_softTabs = nullptr;
    QCheckBox* m_completionEnabled = nullptr;
    QCheckBox* m_syntaxHintsEnabled = nullptr;

    // appearance page
    QComboBox* m_theme = nullptr;
    QCheckBox* m_hideUnused = nullptr;

    // interface page
    QComboBox* m_language = nullptr;
    QLabel* m_languageNote = nullptr;
};

}  // namespace spall
