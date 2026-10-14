// SettingsDialog.cpp — settings persistence and editing UI.
#include "SettingsDialog.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFontComboBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QSettings>
#include <QSpinBox>
#include <QTabWidget>
#include <QVBoxLayout>

#include "Translator.h"

namespace spall {

namespace {

/// QSettings keys, kept in one place so load() and save() cannot drift apart.
namespace key {
constexpr auto kGlslVersion = "compile/glslVersion";
constexpr auto kVulkanVersion = "compile/vulkanVersion";
constexpr auto kTargetEnv = "compile/targetEnv";
constexpr auto kSpirvVersion = "compile/spirvVersion";
constexpr auto kAutoMapLocations = "compile/autoMapLocations";
constexpr auto kAutoMapBindings = "compile/autoMapBindings";
constexpr auto kEnableIncludes = "compile/enableIncludes";
constexpr auto kValidate = "compile/validate";
constexpr auto kEnablePreamble = "compile/enablePreamble";
constexpr auto kIncludePaths = "compile/includePaths";
constexpr auto kOptimizationLevel = "compile/optimizationLevel";
constexpr auto kDisasmFlags = "compile/disasmFlags";
constexpr auto kFontFamily = "editor/fontFamily";
constexpr auto kFontSize = "editor/fontSize";
constexpr auto kTabWidth = "editor/tabWidth";
constexpr auto kSoftTabs = "editor/softTabs";
constexpr auto kCompletion = "editor/completionEnabled";
constexpr auto kSyntaxHints = "editor/syntaxHintsEnabled";
constexpr auto kTheme = "appearance/theme";
constexpr auto kHideUnused = "appearance/hideUnusedReflection";
constexpr auto kLanguage = "interface/language";
}  // namespace key

}  // namespace

AppSettings AppSettings::load() {
    AppSettings settings;
    QSettings store;

    settings.glslVersion = store.value(key::kGlslVersion, settings.glslVersion).toInt();
    settings.vulkanVersion = static_cast<VulkanVersion>(
        store.value(key::kVulkanVersion, static_cast<int>(settings.vulkanVersion)).toInt());
    settings.targetEnv = static_cast<TargetEnv>(
        store.value(key::kTargetEnv, static_cast<int>(settings.targetEnv)).toInt());
    settings.spirvVersion = static_cast<SpirvVersion>(
        store.value(key::kSpirvVersion, static_cast<int>(settings.spirvVersion)).toInt());
    settings.autoMapLocations = store.value(key::kAutoMapLocations, settings.autoMapLocations).toBool();
    settings.autoMapBindings = store.value(key::kAutoMapBindings, settings.autoMapBindings).toBool();
    settings.enableIncludes = store.value(key::kEnableIncludes, settings.enableIncludes).toBool();
    settings.validate = store.value(key::kValidate, settings.validate).toBool();
    settings.enablePreamble = store.value(key::kEnablePreamble, settings.enablePreamble).toBool();
    settings.includePaths = store.value(key::kIncludePaths, settings.includePaths).toStringList();
    settings.optimizationLevel = static_cast<OptimizationLevel>(
        store.value(key::kOptimizationLevel, static_cast<int>(settings.optimizationLevel)).toInt());
    settings.disasmFlags = store.value(key::kDisasmFlags, settings.disasmFlags).toUInt();

    settings.fontFamily = store.value(key::kFontFamily, settings.fontFamily).toString();
    settings.fontSize = store.value(key::kFontSize, settings.fontSize).toInt();
    settings.tabWidth = store.value(key::kTabWidth, settings.tabWidth).toInt();
    settings.softTabs = store.value(key::kSoftTabs, settings.softTabs).toBool();
    settings.completionEnabled = store.value(key::kCompletion, settings.completionEnabled).toBool();
    settings.syntaxHintsEnabled = store.value(key::kSyntaxHints, settings.syntaxHintsEnabled).toBool();

    settings.theme = store.value(key::kTheme, settings.theme).toString();
    settings.hideUnusedReflection =
        store.value(key::kHideUnused, settings.hideUnusedReflection).toBool();
    settings.language = store.value(key::kLanguage, settings.language).toString();

    return settings;
}

void AppSettings::save() const {
    QSettings store;
    store.setValue(key::kGlslVersion, glslVersion);
    store.setValue(key::kVulkanVersion, static_cast<int>(vulkanVersion));
    store.setValue(key::kTargetEnv, static_cast<int>(targetEnv));
    store.setValue(key::kSpirvVersion, static_cast<int>(spirvVersion));
    store.setValue(key::kAutoMapLocations, autoMapLocations);
    store.setValue(key::kAutoMapBindings, autoMapBindings);
    store.setValue(key::kEnableIncludes, enableIncludes);
    store.setValue(key::kValidate, validate);
    store.setValue(key::kEnablePreamble, enablePreamble);
    store.setValue(key::kIncludePaths, includePaths);
    store.setValue(key::kOptimizationLevel, static_cast<int>(optimizationLevel));
    store.setValue(key::kDisasmFlags, disasmFlags);

    store.setValue(key::kFontFamily, fontFamily);
    store.setValue(key::kFontSize, fontSize);
    store.setValue(key::kTabWidth, tabWidth);
    store.setValue(key::kSoftTabs, softTabs);
    store.setValue(key::kCompletion, completionEnabled);
    store.setValue(key::kSyntaxHints, syntaxHintsEnabled);

    store.setValue(key::kTheme, theme);
    store.setValue(key::kHideUnused, hideUnusedReflection);
    store.setValue(key::kLanguage, language);
    store.sync();
}

CompileOptions AppSettings::toCompileOptions(ShaderStage stage) const {
    CompileOptions options;
    options.stage = stage;
    options.glslVersion = glslVersion;
    options.vulkanVersion = vulkanVersion;
    options.spirvVersion = spirvVersion;
    options.targetEnv = targetEnv;
    options.autoMapLocations = autoMapLocations;
    options.autoMapBindings = autoMapBindings;
    options.enableIncludes = enableIncludes;
    options.includePaths = includePaths;
    options.validate = validate;
    options.enablePreamble = enablePreamble;
    options.limits = limits;
    return options;
}

void AppSettings::applyFrom(const CompileOptions& options) {
    glslVersion = options.glslVersion;
    vulkanVersion = options.vulkanVersion;
    spirvVersion = options.spirvVersion;
    targetEnv = options.targetEnv;
    autoMapLocations = options.autoMapLocations;
    autoMapBindings = options.autoMapBindings;
    enableIncludes = options.enableIncludes;
    includePaths = options.includePaths;
    validate = options.validate;
    enablePreamble = options.enablePreamble;
    limits = options.limits;
}

// --- dialog ------------------------------------------------------------------

SettingsDialog::SettingsDialog(const AppSettings& settings, QWidget* parent)
    : QDialog(parent), m_settings(settings) {
    setWindowTitle(tr("Settings"));
    resize(560, 520);

    auto* layout = new QVBoxLayout(this);
    m_tabs = new QTabWidget(this);
    m_tabs->addTab(buildCompilePage(), tr("Compilation"));
    m_tabs->addTab(buildEditorPage(), tr("Editor"));
    m_tabs->addTab(buildAppearancePage(), tr("Appearance"));
    m_tabs->addTab(buildInterfacePage(), tr("Interface"));
    layout->addWidget(m_tabs, 1);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &SettingsDialog::onAccept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
}

QWidget* SettingsDialog::buildCompilePage() {
    auto* page = new QWidget(this);
    auto* form = new QFormLayout(page);

    m_glslVersion = new QSpinBox(page);
    m_glslVersion->setRange(100, 460);
    m_glslVersion->setSingleStep(10);
    m_glslVersion->setValue(m_settings.glslVersion);
    form->addRow(tr("Default #version"), m_glslVersion);

    m_vulkanVersion = new QComboBox(page);
    for (const VulkanVersion version : {VulkanVersion::Vulkan10, VulkanVersion::Vulkan11,
                                        VulkanVersion::Vulkan12, VulkanVersion::Vulkan13}) {
        m_vulkanVersion->addItem(vulkanVersionName(version), static_cast<int>(version));
    }
    m_vulkanVersion->setCurrentIndex(m_vulkanVersion->findData(static_cast<int>(m_settings.vulkanVersion)));
    form->addRow(tr("Vulkan client version"), m_vulkanVersion);

    m_targetEnv = new QComboBox(page);
    for (const TargetEnv env : {TargetEnv::Vulkan10, TargetEnv::Vulkan11, TargetEnv::Vulkan12,
                                TargetEnv::Vulkan13, TargetEnv::Universal10, TargetEnv::Universal13,
                                TargetEnv::Universal16}) {
        m_targetEnv->addItem(targetEnvName(env), static_cast<int>(env));
    }
    m_targetEnv->setCurrentIndex(m_targetEnv->findData(static_cast<int>(m_settings.targetEnv)));
    form->addRow(tr("Target environment (validation/disassembly)"), m_targetEnv);

    m_spirvVersion = new QComboBox(page);
    for (const SpirvVersion version :
         {SpirvVersion::Spv10, SpirvVersion::Spv11, SpirvVersion::Spv12, SpirvVersion::Spv13,
          SpirvVersion::Spv14, SpirvVersion::Spv15, SpirvVersion::Spv16}) {
        m_spirvVersion->addItem(spirvVersionName(version), static_cast<int>(version));
    }
    m_spirvVersion->setCurrentIndex(m_spirvVersion->findData(static_cast<int>(m_settings.spirvVersion)));
    form->addRow(tr("SPIR-V target version"), m_spirvVersion);

    m_autoMapLocations = new QCheckBox(tr("Assign locations automatically"), page);
    m_autoMapLocations->setChecked(m_settings.autoMapLocations);
    m_autoMapLocations->setToolTip(
        tr("Also numbers IO variables that lack layout(location=), so the module still "
           "validates."));
    form->addRow(QString(), m_autoMapLocations);

    m_autoMapBindings = new QCheckBox(tr("Assign bindings/sets automatically"), page);
    m_autoMapBindings->setChecked(m_settings.autoMapBindings);
    m_autoMapBindings->setToolTip(
        tr("When this is off and the source has no layout(binding=), the module will fail "
           "SPIR-V validation — useful for exercising the validation path."));
    form->addRow(QString(), m_autoMapBindings);

    m_enableIncludes = new QCheckBox(tr("Expand #include directives"), page);
    m_enableIncludes->setChecked(m_settings.enableIncludes);
    form->addRow(QString(), m_enableIncludes);

    m_validate = new QCheckBox(tr("Validate SPIR-V by default"), page);
    m_validate->setChecked(m_settings.validate);
    form->addRow(QString(), m_validate);

    m_enablePreamble = new QCheckBox(tr("Inject tool macro (#define SPALL 1)"), page);
    m_enablePreamble->setChecked(m_settings.enablePreamble);
    form->addRow(QString(), m_enablePreamble);

    m_includePaths = new QPlainTextEdit(page);
    m_includePaths->setPlaceholderText(tr("One directory per line"));
    m_includePaths->setPlainText(m_settings.includePaths.join(QLatin1Char('\n')));
    m_includePaths->setMaximumHeight(80);
    form->addRow(tr("#include search paths"), m_includePaths);

    return page;
}

QWidget* SettingsDialog::buildEditorPage() {
    auto* page = new QWidget(this);
    auto* form = new QFormLayout(page);

    m_fontFamily = new QLineEdit(page);
    m_fontFamily->setText(m_settings.fontFamily);
    m_fontFamily->setPlaceholderText(tr("Leave empty to use the system monospace font"));
    form->addRow(tr("Font"), m_fontFamily);

    m_fontSize = new QSpinBox(page);
    m_fontSize->setRange(6, 48);
    m_fontSize->setValue(m_settings.fontSize);
    form->addRow(tr("Font size"), m_fontSize);

    m_tabWidth = new QSpinBox(page);
    m_tabWidth->setRange(1, 16);
    m_tabWidth->setValue(m_settings.tabWidth);
    form->addRow(tr("Tab width"), m_tabWidth);

    m_softTabs = new QCheckBox(tr("Insert spaces instead of tabs"), page);
    m_softTabs->setChecked(m_settings.softTabs);
    form->addRow(QString(), m_softTabs);

    m_completionEnabled = new QCheckBox(tr("Show completion popup while typing"), page);
    m_completionEnabled->setChecked(m_settings.completionEnabled);
    form->addRow(QString(), m_completionEnabled);

    m_syntaxHintsEnabled = new QCheckBox(tr("Enable basic syntax hints (heuristic)"), page);
    m_syntaxHintsEnabled->setChecked(m_settings.syntaxHintsEnabled);
    m_syntaxHintsEnabled->setToolTip(
        tr("Checks run before compilation: bracket matching, missing semicolons, common "
           "typos. They are reported as hints and warnings only, not as full semantic "
           "analysis."));
    form->addRow(QString(), m_syntaxHintsEnabled);

    return page;
}

QWidget* SettingsDialog::buildAppearancePage() {
    auto* page = new QWidget(this);
    auto* form = new QFormLayout(page);

    m_theme = new QComboBox(page);
    m_theme->addItem(tr("Follow system"), QStringLiteral("system"));
    m_theme->addItem(tr("Light"), QStringLiteral("light"));
    m_theme->addItem(tr("Dark"), QStringLiteral("dark"));
    m_theme->setCurrentIndex(std::max(0, m_theme->findData(m_settings.theme)));
    form->addRow(tr("Theme"), m_theme);

    m_hideUnused = new QCheckBox(tr("Hide unused variables in the reflection panel"), page);
    m_hideUnused->setChecked(m_settings.hideUnusedReflection);
    form->addRow(QString(), m_hideUnused);

    auto* note = new QLabel(
        tr("Window layout, geometry and the recent-files list are saved automatically and "
           "restored on the next start."),
        page);
    note->setWordWrap(true);
    form->addRow(note);

    return page;
}

QWidget* SettingsDialog::buildInterfacePage() {
    auto* page = new QWidget(this);
    auto* form = new QFormLayout(page);

    m_language = new QComboBox(page);
    // The language names are deliberately *not* translated: each entry is written in its
    // own language so it is readable by someone who cannot read the current one.
    for (const LanguageInfo& info : Translator::available()) {
        m_language->addItem(QString::fromUtf8(info.native),
                            Translator::codeForLanguage(info.language));
    }
    m_language->setCurrentIndex(std::max(0, m_language->findData(m_settings.language)));
    form->addRow(tr("Language"), m_language);

    m_languageNote = new QLabel(
        tr("The interface switches immediately; no restart is required. English is the "
           "source language and is always available."),
        page);
    m_languageNote->setWordWrap(true);
    form->addRow(m_languageNote);

    return page;
}

void SettingsDialog::onAccept() {
    m_settings.glslVersion = m_glslVersion->value();
    m_settings.vulkanVersion =
        static_cast<VulkanVersion>(m_vulkanVersion->currentData().toInt());
    m_settings.targetEnv = static_cast<TargetEnv>(m_targetEnv->currentData().toInt());
    m_settings.spirvVersion = static_cast<SpirvVersion>(m_spirvVersion->currentData().toInt());
    m_settings.autoMapLocations = m_autoMapLocations->isChecked();
    m_settings.autoMapBindings = m_autoMapBindings->isChecked();
    m_settings.enableIncludes = m_enableIncludes->isChecked();
    m_settings.validate = m_validate->isChecked();
    m_settings.enablePreamble = m_enablePreamble->isChecked();

    m_settings.includePaths.clear();
    const QStringList lines = m_includePaths->toPlainText().split(QLatin1Char('\n'));
    for (const QString& line : lines) {
        const QString trimmed = line.trimmed();
        if (!trimmed.isEmpty()) {
            m_settings.includePaths << trimmed;
        }
    }

    m_settings.fontFamily = m_fontFamily->text().trimmed();
    m_settings.fontSize = m_fontSize->value();
    m_settings.tabWidth = m_tabWidth->value();
    m_settings.softTabs = m_softTabs->isChecked();
    m_settings.completionEnabled = m_completionEnabled->isChecked();
    m_settings.syntaxHintsEnabled = m_syntaxHintsEnabled->isChecked();

    m_settings.theme = m_theme->currentData().toString();
    m_settings.hideUnusedReflection = m_hideUnused->isChecked();

    m_settings.language = m_language->currentData().toString();

    accept();
}

}  // namespace spall
