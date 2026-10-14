// MainWindow.cpp — application shell implementation.
#include "MainWindow.h"

#include <QAbstractItemView>
#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QCloseEvent>
#include <QComboBox>
#include <QDir>
#include <QDockWidget>
#include <QEvent>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFutureWatcher>
#include <QKeyEvent>
#include <QKeySequence>
#include <QLabel>
#include <QListWidget>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QRegularExpression>
#include <QSettings>
#include <QStatusBar>
#include <QTabWidget>
#include <QTextBrowser>
#include <QTextCursor>
#include <QTextStream>
#include <QToolBar>
#include <QUrl>
#include <QVBoxLayout>
#include <QtConcurrent>

#include <algorithm>
#include <cstdio>

#include "EditorTab.h"
#include "FileBrowser.h"
#include "GlslCompleter.h"
#include "GlslHighlighter.h"
#include "GlslLanguageData.h"
#include "GlslCompiler.h"
#include "SpirvDisassembler.h"
#include "SpirvReflection.h"
#include "StartupTrace.h"
#include "Translator.h"
#include "panels/AsmPanel.h"
#include "panels/DiagnosticsPanel.h"
#include "panels/OutputPanel.h"
#include "panels/ReflectPanel.h"

namespace spall {

namespace {

constexpr int kMaxRecentFiles = 10;

/// Severity ordering helper.
bool severityLess(Severity a, Severity b) {
    return static_cast<int>(a) < static_cast<int>(b);
}

/// Read a UTF-8 text file; returns false and sets `error` on failure.
bool readTextFile(const QString& path, QString* out, QString* error) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        *error = file.errorString();
        return false;
    }
    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);
    *out = stream.readAll();
    return true;
}

bool writeTextFile(const QString& path, const QString& text, QString* error) {
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        *error = file.errorString();
        return false;
    }
    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);
    stream << text;
    return true;
}

}  // namespace

// --- construction ------------------------------------------------------------

namespace {
/// Language forced by `--lang=<code>` for this process, or empty for the saved setting.
QString g_languageOverride;
}  // namespace

void MainWindow::setLanguageOverride(const QString& code) {
    g_languageOverride = code;
}

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    const auto mark = [](const char* what) { startupTrace(what); };
    mark("ctor: enter");
    m_settings = AppSettings::load();
    mark("ctor: settings loaded");

    // Install the saved UI language before anything is built, so the first frame is
    // already in the right language instead of flashing English and then switching.
    // The catalogue is loaded here; the LanguageChange broadcast that re-applies strings
    // happens at the end of the constructor (see below), once every member exists.
    m_translator = new Translator(qApp, this);
    mark("ctor: translator created");

    m_tabs = new QTabWidget(this);
    m_tabs->setTabsClosable(true);
    m_tabs->setMovable(true);
    m_tabs->setDocumentMode(true);
    setCentralWidget(m_tabs);
    mark("ctor: tabs created");

    m_watcher = new QFutureWatcher<CompileResult>(this);
    connect(m_watcher, &QFutureWatcher<CompileResult>::finished, this,
            &MainWindow::onCompileFinished);

    buildActions();
    mark("ctor: actions built");
    buildToolbar();
    mark("ctor: toolbar built");
    buildDocks();
    mark("ctor: docks built");
    buildStatusBar();
    mark("ctor: status bar built");
    buildMenuBar();
    mark("ctor: menu bar built");

    connect(m_tabs, &QTabWidget::currentChanged, this, &MainWindow::onTabChanged);
    connect(m_tabs, &QTabWidget::tabCloseRequested, this, &MainWindow::onCloseTab);

    QSettings store;
    m_recentFiles = store.value(QStringLiteral("files/recent")).toStringList();
    rebuildRecentMenu();
    mark("ctor: recent menu rebuilt");

    restoreLayout();
    mark("ctor: layout restored");
    applyAppearance();
    mark("ctor: appearance applied");
    loadExamples();
    mark("ctor: examples loaded");

    onNewFile();
    mark("ctor: new file created");
    refreshTitles();
    mark("ctor: titles refreshed");
    updateActionState();
    mark("ctor: action state updated");

    m_statusLabel->setText(tr("Ready"));
    mark("ctor: done");

    // Install the saved UI language last. Translator::install() broadcasts
    // QEvent::LanguageChange, and this object is already in QApplication::allWidgets()
    // while its constructor runs — so installing earlier would deliver the event to a
    // half-built window and retranslate() would touch members that do not exist yet.
    m_translator->install(Translator::languageFromCode(g_languageOverride.isEmpty()
                                                           ? m_settings.language
                                                           : g_languageOverride));
    mark("ctor: translator installed");
}

MainWindow::~MainWindow() = default;

// --- actions -----------------------------------------------------------------

void MainWindow::buildActions() {
    // Texts are set here and re-applied by retranslate(); every action that is visible
    // to the user is therefore a member rather than a local.
    m_compileAction = new QAction(tr("Compile"), this);
    m_compileAction->setShortcut(QKeySequence(Qt::Key_F7));
    m_compileAction->setShortcutContext(Qt::ApplicationShortcut);
    m_compileAction->setToolTip(tr("Compile the current tab (F7 / Ctrl+B)"));
    connect(m_compileAction, &QAction::triggered, this, &MainWindow::onCompile);
    addAction(m_compileAction);

    auto* compileAlt = new QAction(this);
    compileAlt->setShortcut(QKeySequence(QStringLiteral("Ctrl+B")));
    compileAlt->setShortcutContext(Qt::ApplicationShortcut);
    connect(compileAlt, &QAction::triggered, this, &MainWindow::onCompile);
    addAction(compileAlt);

    m_compileAllAction = new QAction(tr("Compile All"), this);
    m_compileAllAction->setShortcut(QKeySequence(QStringLiteral("Ctrl+Shift+B")));
    m_compileAllAction->setShortcutContext(Qt::ApplicationShortcut);
    connect(m_compileAllAction, &QAction::triggered, this, &MainWindow::onCompileAll);
    addAction(m_compileAllAction);

    m_validateAction = new QAction(tr("Validate"), this);
    m_validateAction->setToolTip(tr("Re-run SPIR-V validation on the current module"));
    connect(m_validateAction, &QAction::triggered, this, &MainWindow::onValidateRequested);

    m_disasmAction = new QAction(tr("Disassemble"), this);
    m_disasmAction->setToolTip(tr("Disassemble again with the current options"));
    connect(m_disasmAction, &QAction::triggered, this, &MainWindow::onDisassembleRequested);

    m_saveAction = new QAction(tr("Save"), this);
    m_saveAction->setShortcut(QKeySequence::Save);
    connect(m_saveAction, &QAction::triggered, this, &MainWindow::onSave);

    m_saveAsAction = new QAction(tr("Save As..."), this);
    m_saveAsAction->setShortcut(QKeySequence::SaveAs);
    connect(m_saveAsAction, &QAction::triggered, this, &MainWindow::onSaveAs);

    m_reloadAction = new QAction(tr("Reload from Disk"), this);
    connect(m_reloadAction, &QAction::triggered, this, &MainWindow::onReload);

    m_newAction = new QAction(tr("New"), this);
    m_newAction->setShortcut(QKeySequence::New);
    connect(m_newAction, &QAction::triggered, this, &MainWindow::onNewFile);

    m_openAction = new QAction(tr("Open..."), this);
    m_openAction->setShortcut(QKeySequence::Open);
    connect(m_openAction, &QAction::triggered, this, &MainWindow::onOpenFile);

    auto* nextDiag = new QAction(this);
    nextDiag->setShortcut(QKeySequence(Qt::Key_F8));
    nextDiag->setShortcutContext(Qt::ApplicationShortcut);
    connect(nextDiag, &QAction::triggered, this, &MainWindow::onNextDiagnostic);
    addAction(nextDiag);

    auto* prevDiag = new QAction(this);
    prevDiag->setShortcut(QKeySequence(QStringLiteral("Shift+F8")));
    prevDiag->setShortcutContext(Qt::ApplicationShortcut);
    connect(prevDiag, &QAction::triggered, this, &MainWindow::onPreviousDiagnostic);
    addAction(prevDiag);

    m_exportAssemblyAction = new QAction(tr("Export Assembly..."), this);
    m_exportAssemblyAction->setShortcut(QKeySequence(QStringLiteral("Ctrl+E")));
    m_exportAssemblyAction->setShortcutContext(Qt::ApplicationShortcut);
    connect(m_exportAssemblyAction, &QAction::triggered, this, &MainWindow::onExportAssembly);
    addAction(m_exportAssemblyAction);

    m_copyAssemblyAction = new QAction(tr("Copy Assembly"), this);
    m_copyAssemblyAction->setShortcut(QKeySequence(QStringLiteral("Ctrl+Shift+C")));
    m_copyAssemblyAction->setShortcutContext(Qt::ApplicationShortcut);
    connect(m_copyAssemblyAction, &QAction::triggered, this, &MainWindow::onCopyAssembly);
    addAction(m_copyAssemblyAction);

    // Save the compiled module itself. Ctrl+Shift+E mirrors Ctrl+E for the text
    // form, so the two exports sit next to each other.
    m_saveSpvAction = new QAction(tr("Save SPIR-V (.spv)"), this);
    m_saveSpvAction->setShortcut(QKeySequence(QStringLiteral("Ctrl+Shift+E")));
    m_saveSpvAction->setShortcutContext(Qt::ApplicationShortcut);
    m_saveSpvAction->setToolTip(tr("Write the compiled SPIR-V binary to a .spv file "
                                   "(Ctrl+Shift+E)"));
    connect(m_saveSpvAction, &QAction::triggered, this, &MainWindow::onExportBinary);
    addAction(m_saveSpvAction);

    m_settingsAction = new QAction(tr("Settings..."), this);
    connect(m_settingsAction, &QAction::triggered, this, &MainWindow::onSettings);

    m_aboutAction = new QAction(tr("About SpAll"), this);
    connect(m_aboutAction, &QAction::triggered, this, &MainWindow::onAbout);

    // Right-hand tabs: Ctrl+1..4.
    for (int i = 1; i <= 4; ++i) {
        auto* action = new QAction(this);
        action->setShortcut(QKeySequence(QStringLiteral("Ctrl+%1").arg(i)));
        action->setShortcutContext(Qt::ApplicationShortcut);
        connect(action, &QAction::triggered, this, [this, i]() {
            if (m_rightTabs != nullptr && i - 1 < m_rightTabs->count()) {
                m_rightTabs->setCurrentIndex(i - 1);
            }
        });
        addAction(action);
    }
}

void MainWindow::buildToolbar() {
    m_toolBar = addToolBar(tr("Main Toolbar"));
    m_toolBar->setMovable(false);

    m_toolBar->addAction(m_compileAction);
    m_toolBar->addAction(m_saveAction);
    m_toolBar->addSeparator();

    m_stageLabel = new QLabel(tr(" Stage "), m_toolBar);
    m_toolBar->addWidget(m_stageLabel);
    m_stageCombo = new QComboBox(m_toolBar);
    m_stageCombo->addItems(allStageNames());
    m_stageCombo->setToolTip(tr("Shader stage of the current tab"));
    connect(m_stageCombo, &QComboBox::currentTextChanged, this, &MainWindow::onStageComboChanged);
    m_toolBar->addWidget(m_stageCombo);

    m_versionLabel = new QLabel(tr(" Version "), m_toolBar);
    m_toolBar->addWidget(m_versionLabel);
    m_versionCombo = new QComboBox(m_toolBar);
    for (int version : {100, 110, 120, 130, 140, 150, 330, 400, 410, 420, 430, 440, 450, 460}) {
        m_versionCombo->addItem(QString::number(version), version);
    }
    m_versionCombo->setToolTip(tr("GLSL #version"));
    connect(m_versionCombo, &QComboBox::currentTextChanged, this, &MainWindow::onVersionComboChanged);
    m_toolBar->addWidget(m_versionCombo);

    m_vulkanLabel = new QLabel(tr(" Vulkan "), m_toolBar);
    m_toolBar->addWidget(m_vulkanLabel);
    m_vulkanCombo = new QComboBox(m_toolBar);
    for (const VulkanVersion version : {VulkanVersion::Vulkan10, VulkanVersion::Vulkan11,
                                        VulkanVersion::Vulkan12, VulkanVersion::Vulkan13}) {
        m_vulkanCombo->addItem(vulkanVersionName(version), static_cast<int>(version));
    }
    m_vulkanCombo->setToolTip(tr("Target Vulkan version (decides the SPIR-V version and the "
                                 "validation environment)"));
    connect(m_vulkanCombo, &QComboBox::currentIndexChanged, this, [this](int) {
        const auto version = static_cast<VulkanVersion>(m_vulkanCombo->currentData().toInt());
        // Keep SPIR-V version and target env consistent with the Vulkan version.
        m_settings.vulkanVersion = version;
        m_settings.spirvVersion = defaultSpirvVersionFor(version);
        m_settings.targetEnv = defaultTargetEnvFor(version);
        m_settings.save();
        updateActionState();
        refreshStatus();
    });
    m_toolBar->addWidget(m_vulkanCombo);

    m_optimizationLabel = new QLabel(tr(" Optimize "), m_toolBar);
    m_toolBar->addWidget(m_optimizationLabel);
    m_optimizationCombo = new QComboBox(m_toolBar);
    for (const OptimizationLevel level :
         {OptimizationLevel::None, OptimizationLevel::Performance, OptimizationLevel::Size}) {
        m_optimizationCombo->addItem(optimizationLevelName(level), static_cast<int>(level));
    }
    m_optimizationCombo->setToolTip(tr("SPIRV-Tools optimization level"));
    connect(m_optimizationCombo, &QComboBox::currentIndexChanged, this,
            &MainWindow::onOptimizationChanged);
    m_toolBar->addWidget(m_optimizationCombo);

    m_toolBar->addSeparator();
    m_toolBar->addAction(m_validateAction);
    m_toolBar->addAction(m_disasmAction);
    m_toolBar->addAction(m_saveSpvAction);
}

void MainWindow::buildDocks() {
    // --- left: file browser + bundled examples ------------------------------
    m_leftDock = new QDockWidget(tr("Files"), this);

    m_fileBrowser = new FileBrowser(m_leftDock);
    connect(m_fileBrowser, &FileBrowser::fileActivated, this, &MainWindow::openFilePath);

    // The examples list stays below the browser so the shipped samples are always
    // one click away regardless of which folder is being browsed. A QTextBrowser
    // is used rather than a plain text view because it turns the file names into
    // real anchors, so a single click opens them.
    auto* leftPanel = new QWidget(m_leftDock);
    auto* leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(2);

    auto* examplesLabel = new QLabel(QStringLiteral("examples/"), leftPanel);
    m_exampleList = new QTextBrowser(leftPanel);
    m_exampleList->setOpenExternalLinks(false);
    m_exampleList->setOpenLinks(false);  // handled through anchorClicked
    m_exampleList->setMaximumHeight(120);
    m_exampleList->setToolTip(tr("Click a file name to open it"));

    leftLayout->addWidget(examplesLabel);
    leftLayout->addWidget(m_exampleList);
    leftLayout->addWidget(m_fileBrowser, 1);

    connect(m_exampleList, &QTextBrowser::anchorClicked, this, [this](const QUrl& url) {
        openFilePath(url.toLocalFile());
    });

    m_leftDock->setWidget(leftPanel);
    addDockWidget(Qt::LeftDockWidgetArea, m_leftDock);

    // --- right: assembly / reflection / stats+log ---------------------------
    m_rightDock = new QDockWidget(tr("Output"), this);
    m_rightTabs = new QTabWidget(m_rightDock);

    m_asmPanel = new AsmPanel(m_rightTabs);
    m_reflectPanel = new ReflectPanel(m_rightTabs);
    m_outputPanel = new OutputPanel(m_rightTabs);

    m_rightTabs->addTab(m_asmPanel, tr("SPIR-V Assembly"));
    m_rightTabs->addTab(m_reflectPanel, tr("Reflection"));
    m_rightTabs->addTab(m_outputPanel, tr("Stats / Log"));

    connect(m_asmPanel, &AsmPanel::disasmFlagsChanged, this, &MainWindow::onDisasmFlagsChanged);
    connect(m_reflectPanel, &ReflectPanel::hideUnusedChanged, this, [this](bool hide) {
        m_settings.hideUnusedReflection = hide;
        m_settings.save();
    });

    m_rightDock->setWidget(m_rightTabs);
    addDockWidget(Qt::RightDockWidgetArea, m_rightDock);

    // --- bottom: diagnostics ------------------------------------------------
    m_bottomDock = new QDockWidget(tr("Problems"), this);
    m_diagnosticsPanel = new DiagnosticsPanel(m_bottomDock);
    connect(m_diagnosticsPanel, &DiagnosticsPanel::diagnosticActivated, this,
            &MainWindow::onDiagnosticActivated);
    m_bottomDock->setWidget(m_diagnosticsPanel);
    addDockWidget(Qt::BottomDockWidgetArea, m_bottomDock);

    resizeDocks({m_leftDock, m_rightDock}, {200, 560}, Qt::Horizontal);
    resizeDocks({m_bottomDock}, {180}, Qt::Vertical);
}

void MainWindow::buildStatusBar() {
    m_positionLabel = new QLabel(this);
    m_encodingLabel = new QLabel(QStringLiteral("UTF-8"), this);
    m_timeLabel = new QLabel(this);
    m_wordLabel = new QLabel(this);
    m_optimizedWordLabel = new QLabel(this);
    m_hintLabel = new QLabel(this);
    m_statusLabel = new QLabel(this);

    statusBar()->addWidget(m_positionLabel);
    statusBar()->addWidget(m_encodingLabel);
    statusBar()->addPermanentWidget(m_hintLabel, 1);
    statusBar()->addPermanentWidget(m_timeLabel);
    statusBar()->addPermanentWidget(m_wordLabel);
    statusBar()->addPermanentWidget(m_optimizedWordLabel);
    statusBar()->addPermanentWidget(m_statusLabel);
}

// --- layout persistence ------------------------------------------------------

void MainWindow::restoreLayout() {
    QSettings store;
    const QByteArray geometry = store.value(QStringLiteral("window/geometry")).toByteArray();
    if (!geometry.isEmpty()) {
        restoreGeometry(geometry);
    }
    const QByteArray state = store.value(QStringLiteral("window/state")).toByteArray();
    if (!state.isEmpty()) {
        restoreState(state);
    }

    // Right-hand tab selection.
    const int rightTab = store.value(QStringLiteral("window/rightTab"), 0).toInt();
    if (m_rightTabs != nullptr && rightTab >= 0 && rightTab < m_rightTabs->count()) {
        m_rightTabs->setCurrentIndex(rightTab);
    }
}

void MainWindow::saveLayout() {
    QSettings store;
    store.setValue(QStringLiteral("window/geometry"), saveGeometry());
    store.setValue(QStringLiteral("window/state"), saveState());
    if (m_rightTabs != nullptr) {
        store.setValue(QStringLiteral("window/rightTab"), m_rightTabs->currentIndex());
    }
    store.setValue(QStringLiteral("files/recent"), m_recentFiles);
    store.sync();
}

void MainWindow::closeEvent(QCloseEvent* event) {
    for (int i = 0; i < m_tabs->count(); ++i) {
        auto* tab = qobject_cast<EditorTab*>(m_tabs->widget(i));
        if (tab != nullptr && tab->isModified()) {
            m_tabs->setCurrentIndex(i);
            if (!maybeSaveTab(tab)) {
                event->ignore();
                return;
            }
        }
    }
    // Wait for an in-flight compile so the worker does not outlive the window.
    if (m_watcher->isRunning()) {
        m_watcher->waitForFinished();
    }
    saveLayout();
    event->accept();
}

bool MainWindow::maybeSaveTab(EditorTab* tab) {
    if (tab == nullptr || !tab->isModified()) {
        return true;
    }
    const auto choice = QMessageBox::warning(
        this, tr("Unsaved Changes"),
        tr("%1 has unsaved changes.\n\nDo you want to save them?").arg(tab->displayName()),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    if (choice == QMessageBox::Save) {
        onSave();
        return !tab->isModified();
    }
    return choice == QMessageBox::Discard;
}

// --- helpers -----------------------------------------------------------------

EditorTab* MainWindow::currentTab() const {
    return qobject_cast<EditorTab*>(m_tabs->currentWidget());
}

int MainWindow::findTab(const QString& filePath) const {
    for (int i = 0; i < m_tabs->count(); ++i) {
        auto* tab = qobject_cast<EditorTab*>(m_tabs->widget(i));
        if (tab != nullptr && !filePath.isEmpty() && tab->filePath() == filePath) {
            return i;
        }
    }
    return -1;
}

void MainWindow::addRecentFile(const QString& path) {
    if (path.isEmpty()) {
        return;
    }
    m_recentFiles.removeAll(path);
    m_recentFiles.prepend(path);
    while (m_recentFiles.size() > kMaxRecentFiles) {
        m_recentFiles.removeLast();
    }
    rebuildRecentMenu();
}

void MainWindow::buildMenuBar() {
    // Built once, up front, rather than lazily on the first rebuildRecentMenu() call:
    // retranslate() needs stable pointers to every menu, and the Language menu must
    // exist before the first language switch.
    m_fileMenu = menuBar()->addMenu(QString());
    m_fileMenu->addAction(m_newAction);
    m_fileMenu->addAction(m_openAction);
    m_fileMenu->addAction(m_saveAction);
    m_fileMenu->addAction(m_saveAsAction);
    m_fileMenu->addAction(m_reloadAction);
    m_fileMenu->addSeparator();
    m_recentMenu = m_fileMenu->addMenu(QString());

    m_buildMenu = menuBar()->addMenu(QString());
    m_buildMenu->addAction(m_compileAction);
    m_buildMenu->addAction(m_compileAllAction);
    m_buildMenu->addSeparator();
    m_buildMenu->addAction(m_validateAction);
    m_buildMenu->addAction(m_disasmAction);
    m_buildMenu->addSeparator();
    m_buildMenu->addAction(m_exportAssemblyAction);
    m_buildMenu->addAction(m_copyAssemblyAction);
    m_buildMenu->addAction(m_saveSpvAction);
    m_buildMenu->addSeparator();
    m_buildMenu->addAction(m_settingsAction);

    m_viewMenu = menuBar()->addMenu(QString());
    m_viewMenu->addAction(m_leftDock->toggleViewAction());
    m_viewMenu->addAction(m_rightDock->toggleViewAction());
    m_viewMenu->addAction(m_bottomDock->toggleViewAction());
    m_viewMenu->addSeparator();
    buildLanguageMenu();

    m_helpMenu = menuBar()->addMenu(QString());
    m_helpMenu->addAction(m_aboutAction);

    retranslate();
}

void MainWindow::buildLanguageMenu() {
    // The Language menu lives under View: it changes how the window presents itself, so
    // alongside the panel toggles is where a user looks for it.
    m_languageMenu = m_viewMenu->addMenu(QString());
    m_languageGroup = new QActionGroup(this);
    m_languageGroup->setExclusive(true);

    for (const LanguageInfo& info : Translator::available()) {
        // Endonyms on purpose: a native label such as the Simplified Chinese one stays
        // readable when the UI is in English and vice versa, which matters most for the
        // person who cannot read the current language.
        auto* action = new QAction(QString::fromUtf8(info.native), this);
        action->setCheckable(true);
        action->setChecked(info.language == m_translator->current());
        action->setData(Translator::codeForLanguage(info.language));
        connect(action, &QAction::triggered, this, [this, action]() {
            onLanguageSelected(action->data().toString());
        });
        m_languageGroup->addAction(action);
        m_languageMenu->addAction(action);
    }
}

void MainWindow::onLanguageSelected(const QString& code) {
    const Language language = Translator::languageFromCode(code);
    if (language == m_translator->current()) {
        return;
    }
    if (!m_translator->install(language)) {
        QMessageBox::warning(
            this, tr("Language"),
            tr("The translation catalogue for \"%1\" could not be loaded.\n\n"
               "Expected a file named spall_%2.qm next to the executable, in a "
               "translations/ sub-directory. The interface stays in the current "
               "language.")
                .arg(code, code));
        // Put the check mark back where it belongs.
        for (QAction* action : m_languageGroup->actions()) {
            action->setChecked(action->data().toString() ==
                               Translator::codeForLanguage(m_translator->current()));
        }
        return;
    }

    // Persist so the choice survives a restart.
    m_settings.language = code;
    m_settings.save();

    // install() has already sent LanguageChange to every widget, which called
    // retranslate() through changeEvent(). Only the language-dependent strings that no
    // widget owns are left, and those are rebuilt here.
    rebuildRecentMenu();
    refreshTitles();
    refreshStatus();
}

void MainWindow::changeEvent(QEvent* event) {
    if (event->type() == QEvent::LanguageChange) {
        retranslate();
    }
    QMainWindow::changeEvent(event);
}

void MainWindow::retranslate() {
    // retranslate() can be reached at any point in the window's life, including before
    // the constructor has finished, so every member it touches is checked. Skipping a
    // string is always better than dereferencing a half-built window.
    if (m_newAction == nullptr) {
        return;
    }

    // --- menu bar -----------------------------------------------------------
    if (m_fileMenu != nullptr) {
        m_fileMenu->setTitle(tr("&File"));
        m_recentMenu->setTitle(tr("Recent Files"));
        m_buildMenu->setTitle(tr("&Build"));
        m_viewMenu->setTitle(tr("&View"));
        m_languageMenu->setTitle(tr("Language"));
        m_helpMenu->setTitle(tr("&Help"));
    }

    // --- actions ------------------------------------------------------------
    m_newAction->setText(tr("New"));
    m_openAction->setText(tr("Open..."));
    m_saveAction->setText(tr("Save"));
    m_saveAsAction->setText(tr("Save As..."));
    m_reloadAction->setText(tr("Reload from Disk"));
    m_compileAction->setText(tr("Compile"));
    m_compileAction->setToolTip(tr("Compile the current tab (F7 / Ctrl+B)"));
    m_compileAllAction->setText(tr("Compile All"));
    m_validateAction->setText(tr("Validate"));
    m_validateAction->setToolTip(tr("Re-run SPIR-V validation on the current module"));
    m_disasmAction->setText(tr("Disassemble"));
    m_disasmAction->setToolTip(tr("Disassemble again with the current options"));
    m_exportAssemblyAction->setText(tr("Export Assembly..."));
    m_copyAssemblyAction->setText(tr("Copy Assembly"));
    m_saveSpvAction->setText(tr("Save SPIR-V (.spv)"));
    m_saveSpvAction->setToolTip(tr("Write the compiled SPIR-V binary to a .spv file "
                                   "(Ctrl+Shift+E)"));
    m_settingsAction->setText(tr("Settings..."));
    m_aboutAction->setText(tr("About SpAll"));

    // --- toolbar ------------------------------------------------------------
    if (m_toolBar != nullptr) {
        m_toolBar->setWindowTitle(tr("Main Toolbar"));
        m_stageLabel->setText(tr(" Stage "));
        m_stageCombo->setToolTip(tr("Shader stage of the current tab"));
        m_versionLabel->setText(tr(" Version "));
        m_versionCombo->setToolTip(tr("GLSL #version"));
        m_vulkanLabel->setText(tr(" Vulkan "));
        m_vulkanCombo->setToolTip(tr("Target Vulkan version (decides the SPIR-V version and "
                                     "the validation environment)"));
        m_optimizationLabel->setText(tr(" Optimize "));
        m_optimizationCombo->setToolTip(tr("SPIRV-Tools optimization level"));
    }

    // --- docks and their tabs ----------------------------------------------
    if (m_leftDock != nullptr) {
        m_leftDock->setWindowTitle(tr("Files"));
        m_exampleList->setToolTip(tr("Click a file name to open it"));
        m_rightDock->setWindowTitle(tr("Output"));
        m_rightTabs->setTabText(0, tr("SPIR-V Assembly"));
        m_rightTabs->setTabText(1, tr("Reflection"));
        m_rightTabs->setTabText(2, tr("Stats / Log"));
        m_bottomDock->setWindowTitle(tr("Problems"));

        // The panel toggles in the View menu take their text from the dock titles, but
        // only when the dock title changes: re-apply them explicitly.
        m_leftDock->toggleViewAction()->setText(tr("Files"));
        m_rightDock->toggleViewAction()->setText(tr("Output"));
        m_bottomDock->toggleViewAction()->setText(tr("Problems"));
    }

    // Child widgets (docked panels are not top-level) never receive LanguageChange on
    // their own, so they are told here.
    triggerRetranslate();

    // Status bar labels that hold text rather than state.
    if (m_statusLabel != nullptr) {
        m_statusLabel->setText(tr("Ready"));
    }
}

void MainWindow::triggerRetranslate() {
    if (m_fileBrowser != nullptr) {
        m_fileBrowser->retranslate();
    }
    if (m_asmPanel != nullptr) {
        m_asmPanel->retranslate();
    }
    if (m_reflectPanel != nullptr) {
        m_reflectPanel->retranslate();
    }
    if (m_outputPanel != nullptr) {
        m_outputPanel->retranslate();
    }
    if (m_diagnosticsPanel != nullptr) {
        m_diagnosticsPanel->retranslate();
    }

    // Every open editor tab owns a completer whose hints are localised.
    for (int i = 0; i < m_tabs->count(); ++i) {
        if (auto* tab = qobject_cast<EditorTab*>(m_tabs->widget(i))) {
            tab->retranslate();
        }
    }
}

void MainWindow::rebuildRecentMenu() {
    if (m_recentMenu == nullptr) {
        return;
    }
    m_recentMenu->clear();
    if (m_recentFiles.isEmpty()) {
        m_recentMenu->addAction(tr("(none)"))->setEnabled(false);
        return;
    }
    for (const QString& path : m_recentFiles) {
        m_recentMenu->addAction(path, this, &MainWindow::onOpenRecent);
    }
}

void MainWindow::refreshTitles() {
    EditorTab* tab = currentTab();
    QString title = QStringLiteral("SpAll");
    if (tab != nullptr) {
        title = QStringLiteral("%1%2 — SpAll").arg(tab->isModified() ? QStringLiteral("*") : QString(),
                                                   tab->displayName());
    }
    setWindowTitle(title);

    // Tab labels: name, dirty marker, staleness.
    m_updatingTabs = true;
    for (int i = 0; i < m_tabs->count(); ++i) {
        auto* each = qobject_cast<EditorTab*>(m_tabs->widget(i));
        if (each == nullptr) {
            continue;
        }
        QString label = each->displayName();
        if (each->isModified()) {
            label.prepend(QLatin1Char('*'));
        }
        if (each->isStale() && !each->isModified() && each->filePath().isEmpty()) {
            // Untitled and never compiled: no marker needed.
        }
        m_tabs->setTabText(i, label);
        const QString tip = each->filePath().isEmpty()
                                ? tr("Unsaved new file")
                                : each->filePath();
        m_tabs->setTabToolTip(i, tip);
    }
    m_updatingTabs = false;

    updateActionState();
}

void MainWindow::refreshStatus() {
    if (m_statusLabel == nullptr) {
        return;
    }
    const auto level = static_cast<OptimizationLevel>(m_optimizationCombo->currentData().toInt());
    m_statusLabel->setText(QStringLiteral("%1 | %2 | %3")
                               .arg(stageName(currentTab() != nullptr ? currentTab()->stage()
                                                                      : ShaderStage::Vertex),
                                    optimizationLevelName(level),
                                    targetEnvName(m_settings.targetEnv)));
}

void MainWindow::updateActionState() {
    const bool hasTab = currentTab() != nullptr;
    m_compileAction->setEnabled(hasTab);
    m_compileAllAction->setEnabled(hasTab);
    m_validateAction->setEnabled(hasTab);
    m_disasmAction->setEnabled(hasTab);
    m_saveAsAction->setEnabled(hasTab);
    m_reloadAction->setEnabled(hasTab && !(currentTab()->filePath().isEmpty()));
    m_saveAction->setEnabled(hasTab && currentTab()->isModified());
    // Saving a module requires a successful compile of the current tab.
    m_saveSpvAction->setEnabled(hasTab && currentTab()->lastResult().success &&
                                !currentTab()->lastResult().spirvOptimized.isEmpty());
}

int MainWindow::runCompletionSelfTest() {
    // Exercises completion on the live window: open a tab, type a prefix through real
    // key events, and check that the popup appeared and offers matching names. This
    // cannot be covered by a standalone widget test because the popup is opened from
    // the editor's textChanged on a window that is really on screen.
    //
    // Findings go to stderr and to a report file next to the executable: SpAll is a
    // GUI-subsystem binary, so a shell frequently cannot see its stderr at all. Each
    // line is flushed, so a run that is killed part-way still shows how far it got.
    QStringList lines;
    const auto report = [&lines](const QString& line) {
        lines << line;
        std::fprintf(stderr, "[selftest] %s\n", line.toUtf8().constData());
        std::fflush(stderr);
    };
    const auto finish = [&lines](int result) {
        QFile file(QCoreApplication::applicationDirPath() +
                   QStringLiteral("/spall-selftest-report.txt"));
        if (file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
            QTextStream stream(&file);
            stream.setEncoding(QStringConverter::Utf8);
            stream << lines.join(QLatin1Char('\n')) << QLatin1Char('\n');
        }
        return result;
    };

    // The self-test types into a document, which marks it modified. app.quit() closes
    // the window, and closeEvent() then raises the modal "Unsaved Changes" prompt — which
    // no one is there to answer, so the process would sit on the dialog forever instead of
    // returning its exit code. The test therefore runs on a scratch tab of its own and
    // throws it away afterwards, which also means a run never disturbs open documents.
    const auto dropTestTab = [this](EditorTab* tab) {
        if (tab == nullptr) {
            return;
        }
        if (QAbstractItemView* popup = tab->completer()->popupView()) {
            popup->hide();
        }
        tab->markClean();
        const int index = m_tabs->indexOf(tab);
        if (index >= 0) {
            m_tabs->removeTab(index);
            tab->deleteLater();
        }
    };

    onNewFile();
    EditorTab* tab = currentTab();
    if (tab == nullptr || tab->editor() == nullptr) {
        report(QStringLiteral("FAIL: could not create an editor tab to test"));
        return finish(1);
    }

    tab->setSource(QStringLiteral("#version 450\nvoid main()\n{\n    normal\n}\n"));
    QPlainTextEdit* editor = tab->editor();

    // Put the caret at the end of "normal" on line 4, the position a user is in
    // while typing an identifier.
    QTextCursor cursor = editor->textCursor();
    cursor.movePosition(QTextCursor::Start);
    cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, 3);
    cursor.movePosition(QTextCursor::EndOfBlock);
    editor->setTextCursor(cursor);
    editor->setFocus();

    // Real key events rather than setPlainText, so the auto-trigger path runs exactly
    // as it does when someone types.
    const QString typed = QStringLiteral("ize");
    for (const QChar ch : typed) {
        QKeyEvent press(QEvent::KeyPress, 0, Qt::NoModifier, QString(ch));
        QApplication::sendEvent(editor, &press);
    }
    QApplication::processEvents();

    // Report what is known before touching the completion model, so a failure inside
    // it still leaves a readable trace.
    const bool visible = tab->completer()->isPopupVisible();
    report(QStringLiteral("typed '%1' after 'normal' -> prefix '%2'")
               .arg(typed, tab->completer()->debugPrefix()));
    report(QStringLiteral("auto-trigger=%1 popup=%2")
               .arg(tab->completer()->autoTriggerEnabled() ? QStringLiteral("on")
                                                           : QStringLiteral("OFF"))
               .arg(visible ? QStringLiteral("visible") : QStringLiteral("HIDDEN")));

    // Report the UI language and whether its catalogue actually loaded. A missing or
    // stale catalogue is otherwise invisible: the window simply stays in English.
    // "Ready" is a string this class itself translates, so it is the right probe —
    // testing a string owned by another context would always look untranslated here.
    const bool translated = tr("Ready") != QLatin1String("Ready");
    report(QStringLiteral("language=%1 catalogue=%2")
               .arg(Translator::codeForLanguage(m_translator->current()))
               .arg(translated ? QStringLiteral("active")
                               : (m_translator->current() == Language::English
                                      ? QStringLiteral("n/a (source language)")
                                      : QStringLiteral("NOT LOADED"))));

    const int candidates = tab->completer()->debugCandidateCount();
    report(QStringLiteral("candidates=%1").arg(candidates));
    report(QStringLiteral("first matches: %1").arg(tab->completer()->debugFirstMatches(3)));

    // Where is the popup actually drawn, relative to the caret? QCompleter takes the
    // popup position from the rect's bottom-left corner in *widget* coordinates and
    // maps it to the screen, so the check is against the caret's own global position.
    QAbstractItemView* popupView = tab->completer()->popupView();
    if (popupView != nullptr && editor->viewport() != nullptr) {
        const QPoint caretTopLeft = editor->viewport()->mapToGlobal(editor->cursorRect().topLeft());
        const QPoint caretBottomLeft =
            editor->viewport()->mapToGlobal(editor->cursorRect().bottomLeft());
        const QPoint popupTopLeft = popupView->mapToGlobal(QPoint(0, 0));
        report(QStringLiteral("caret on screen: topLeft=(%1,%2) bottomLeft=(%3,%4)")
                   .arg(caretTopLeft.x())
                   .arg(caretTopLeft.y())
                   .arg(caretBottomLeft.x())
                   .arg(caretBottomLeft.y()));
        report(QStringLiteral("popup on screen: (%1,%2) size %3x%4")
                   .arg(popupTopLeft.x())
                   .arg(popupTopLeft.y())
                   .arg(popupView->width())
                   .arg(popupView->height()));
        report(QStringLiteral("popup offset from caret bottom-left: dx=%1 dy=%2")
                   .arg(popupTopLeft.x() - caretBottomLeft.x())
                   .arg(popupTopLeft.y() - caretBottomLeft.y()));
    }

    // Does Tab accept the highlighted candidate, or does it fall through to the
    // editor's indentation? The key can arrive by two different routes — the editor
    // when it holds focus, or the popup window when it has grabbed the keyboard — and
    // only one of them was ever exercised, which is how "Tab still indents" survived a
    // passing self-test. Both are checked here.
    {
        const auto tryTab = [&](const QString& route, QWidget* target) {
            editor->setFocus();
            tab->completer()->triggerCompletion();
            QApplication::processEvents();
            if (!tab->completer()->isPopupVisible()) {
                report(QStringLiteral("Tab via %1: popup not visible, skipping").arg(route));
                return true;
            }
            const QString highlighted = tab->completer()->debugCurrentCompletion();
            const QString beforeTab = editor->toPlainText();
            QKeyEvent tabKey(QEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier);
            QApplication::sendEvent(target, &tabKey);
            QApplication::processEvents();
            const QString afterTab = editor->toPlainText();
            report(QStringLiteral("Tab via %1: focusWidget=%2 highlighted='%3' "
                                  "textChanged=%4 containsTab=%5")
                       .arg(route)
                       .arg(QApplication::focusWidget() != nullptr
                                ? QApplication::focusWidget()->metaObject()->className()
                                : "null")
                       .arg(highlighted)
                       .arg(afterTab != beforeTab ? QStringLiteral("yes") : QStringLiteral("no"))
                       .arg(afterTab.contains(QLatin1Char('\t')) ? QStringLiteral("YES")
                                                                 : QStringLiteral("no")));
            const bool ok = !afterTab.contains(QLatin1Char('\t')) && afterTab != beforeTab;
            // Undo whatever happened so the next route starts from the same state.
            editor->document()->setModified(false);
            return ok;
        };

        bool tabOk = tryTab(QStringLiteral("editor"), editor);
        if (QAbstractItemView* popupView = tab->completer()->popupView()) {
            tabOk = tryTab(QStringLiteral("popup window"), popupView) && tabOk;
        }
        if (!tabOk) {
            report(QStringLiteral("FAIL: Tab did not accept the highlighted candidate"));
            dropTestTab(tab);
            return finish(1);
        }
    }

    const bool autoTrigger = tab->completer()->autoTriggerEnabled();
    if (!autoTrigger) {
        report(QStringLiteral("FAIL: auto-trigger is disabled in settings"));
        dropTestTab(tab);
        return finish(1);
    }
    if (!visible) {
        report(QStringLiteral("FAIL: the completion popup did not appear"));
        dropTestTab(tab);
        return finish(1);
    }
    if (candidates == 0) {
        report(QStringLiteral("FAIL: the popup appeared but offered nothing"));
        dropTestTab(tab);
        return finish(1);
    }
    report(QStringLiteral("OK"));
    dropTestTab(tab);
    return finish(0);
}

void MainWindow::onAbout() {    QMessageBox::about(
        this, tr("About SpAll"),
        tr("<b>SpAll %1</b><br><br>"
           "A lightweight GLSL editor with SPIR-V compile/disassemble tools.<br>"
           "glslang and SPIRV-Tools/SPIRV-Reflect are compiled into this process; no "
           "external programs are invoked.<br><br>"
           "F7 compile · Ctrl+Space completion · F8 next problem · Ctrl+1..4 switch right tabs")
            .arg(QApplication::applicationVersion()));
}

// --- file actions ------------------------------------------------------------

void MainWindow::onNewFile() {
    auto* tab = new EditorTab(this);
    // Give a fresh tab a minimal, compilable skeleton so F7 always works.
    tab->setSource(QStringLiteral("#version 450\n\nvoid main()\n{\n    \n}\n"));

    const int index = m_tabs->addTab(tab, tab->displayName());
    m_tabs->setCurrentIndex(index);

    connect(tab, &EditorTab::modificationChanged, this, [this]() { refreshTitles(); });
    connect(tab, &EditorTab::titleChanged, this, [this]() { refreshTitles(); });
    connect(tab, &EditorTab::cursorMoved, this, [this](int line, int column) {
        m_positionLabel->setText(tr("Line %1, Column %2").arg(line).arg(column));
    });
    connect(tab, &EditorTab::parameterHintChanged, this,
            [this](const QString& hint) { m_hintLabel->setText(hint); });

    tab->applyTheme(HighlightTheme::byName(m_settings.theme == QLatin1String("dark")
                                               ? QStringLiteral("dark")
                                               : QStringLiteral("light")));
    tab->applyFont(m_settings.fontFamily, m_settings.fontSize);
    tab->editor()->setIndentSettings(m_settings.tabWidth, m_settings.softTabs);
    tab->completer()->setAutoTriggerEnabled(m_settings.completionEnabled);

    refreshTitles();
    onTabChanged(index);
}

void MainWindow::onOpenFile() {
    const QString path = QFileDialog::getOpenFileName(
        this, tr("Open Shader"), m_fileBrowser != nullptr ? m_fileBrowser->rootPath()
                                                          : QString(),
        FileBrowser::fileDialogFilter());
    if (path.isEmpty()) {
        return;
    }
    openFilePath(path);
}

void MainWindow::openFilePath(const QString& path) {
    if (path.isEmpty()) {
        return;
    }

    // Reuse the tab when the file is already open.
    const int existing = findTab(path);
    if (existing >= 0) {
        m_tabs->setCurrentIndex(existing);
        if (m_fileBrowser != nullptr) {
            m_fileBrowser->revealFile(path);
        }
        return;
    }

    QString text;
    QString error;
    if (!readTextFile(path, &text, &error)) {
        QMessageBox::warning(this, tr("Open Failed"),
                             tr("Could not read %1: %2\n\nCheck the file permissions and make "
                                "sure the file is not locked by another program.")
                                 .arg(path, error));
        return;
    }

    onNewFile();
    auto* tab = currentTab();
    if (tab == nullptr) {
        return;
    }
    tab->setFilePath(path);
    tab->setSource(text);

    // Pick the stage from the extension; a .glsl/.inc header has none, so keep the
    // stage the new tab was created with.
    ShaderStage stage = ShaderStage::Vertex;
    if (stageFromFileName(path, &stage)) {
        tab->setStage(stage);
        m_stageCombo->setCurrentText(stageName(stage));
    }

    addRecentFile(path);
    if (m_fileBrowser != nullptr) {
        m_fileBrowser->revealFile(path);
    }
    refreshTitles();
    m_statusLabel->setText(tr("Opened %1").arg(tab->displayName()));
}

void MainWindow::onOpenRecent() {
    auto* action = qobject_cast<QAction*>(sender());
    if (action == nullptr) {
        return;
    }
    const QString path = action->text();
    if (!QFileInfo::exists(path)) {
        QMessageBox::warning(this, tr("Open Failed"),
                             tr("The file does not exist: %1\n\nIt may have been moved or "
                                "deleted, so the entry has been removed from the recent "
                                "files list.")
                                 .arg(path));
        m_recentFiles.removeAll(path);
        rebuildRecentMenu();
        return;
    }
    openFilePath(path);
}

void MainWindow::onSave() {
    EditorTab* tab = currentTab();
    if (tab == nullptr) {
        return;
    }
    if (tab->filePath().isEmpty()) {
        onSaveAs();
        return;
    }
    QString error;
    if (!writeTextFile(tab->filePath(), tab->source(), &error)) {
        QMessageBox::warning(this, tr("Save Failed"),
                             tr("Could not write %1: %2\n\nCheck whether the file is "
                                "read-only or locked by another program.")
                                 .arg(tab->filePath(), error));
        return;
    }
    tab->markClean();
    addRecentFile(tab->filePath());
    refreshTitles();
    m_statusLabel->setText(tr("Saved"));
}

void MainWindow::onSaveAs() {
    EditorTab* tab = currentTab();
    if (tab == nullptr) {
        return;
    }
    QString suggested = tab->filePath();
    if (suggested.isEmpty()) {
        suggested = QStringLiteral("shader%1").arg(stageFileSuffix(tab->stage()));
    }
    const QString path = QFileDialog::getSaveFileName(this, tr("Save As"), suggested,
                                                      tr("Shaders (*);;All Files (*)"));
    if (path.isEmpty()) {
        return;
    }
    QString error;
    if (!writeTextFile(path, tab->source(), &error)) {
        QMessageBox::warning(this, tr("Save As Failed"),
                             tr("Could not write %1: %2").arg(path, error));
        return;
    }
    tab->setFilePath(path);
    tab->markClean();
    ShaderStage stage = ShaderStage::Vertex;
    if (stageFromFileName(path, &stage)) {
        tab->setStage(stage);
    }
    addRecentFile(path);
    refreshTitles();
    m_statusLabel->setText(tr("Saved"));
}

void MainWindow::onReload() {
    EditorTab* tab = currentTab();
    if (tab == nullptr || tab->filePath().isEmpty()) {
        return;
    }
    if (tab->isModified()) {
        const auto choice = QMessageBox::question(
            this, tr("Reload"),
            tr("Reloading discards the current unsaved changes. Continue?"),
            QMessageBox::Yes | QMessageBox::No);
        if (choice != QMessageBox::Yes) {
            return;
        }
    }
    QString text;
    QString error;
    if (!readTextFile(tab->filePath(), &text, &error)) {
        QMessageBox::warning(this, tr("Reload Failed"),
                             tr("Could not read %1: %2").arg(tab->filePath(), error));
        return;
    }
    tab->setSource(text);
    tab->clearLastResult();
    m_asmPanel->setAssembly(QString(), true);
    m_reflectPanel->setReflection(ReflectData{}, false);
    m_diagnosticsPanel->clear();
    m_diagnostics.clear();
    refreshTitles();
    m_statusLabel->setText(tr("Reloaded"));
}

void MainWindow::onCloseTab(int index) {
    auto* tab = qobject_cast<EditorTab*>(m_tabs->widget(index));
    if (tab == nullptr) {
        return;
    }
    if (!maybeSaveTab(tab)) {
        return;
    }
    m_tabs->removeTab(index);
    tab->deleteLater();
    refreshTitles();
}

// --- compile -----------------------------------------------------------------

CompileOptions MainWindow::optionsForTab(const EditorTab* tab) const {
    CompileOptions options = m_settings.toCompileOptions(tab != nullptr ? tab->stage()
                                                                        : ShaderStage::Vertex);
    // The Vulkan combo is authoritative for the target triple: Vulkan version,
    // SPIR-V version and validation environment must stay consistent.
    options.vulkanVersion = static_cast<VulkanVersion>(m_vulkanCombo->currentData().toInt());
    options.spirvVersion = defaultSpirvVersionFor(options.vulkanVersion);
    options.targetEnv = defaultTargetEnvFor(options.vulkanVersion);
    options.glslVersion = m_versionCombo->currentData().toInt();
    options.optimizationLevel =
        static_cast<OptimizationLevel>(m_optimizationCombo->currentData().toInt());
    options.disasmFlags = m_asmPanel->disasmFlags();
    return options;
}

void MainWindow::startCompile(const QString& filePath, const QString& source) {
    if (m_watcher->isRunning()) {
        // A compile is already in flight; wait for it so at most one worker runs.
        m_watcher->waitForFinished();
    }

    ++m_generation;
    m_runningGeneration = m_generation;
    m_runningFile = filePath;

    m_pendingSource = std::make_shared<QString>(source);
    m_pendingOptions = std::make_shared<CompileOptions>(optionsForTab(currentTab()));

    const auto sourcePtr = m_pendingSource;
    const auto optionsPtr = m_pendingOptions;

    m_statusLabel->setText(tr("Compiling..."));

    m_watcher->setFuture(QtConcurrent::run([sourcePtr, optionsPtr, filePath]() {
        // Runs on a worker thread: glslang + SPIRV-Tools are used single-threaded
        // per call, and the result is a self-contained value.
        GlslCompiler compiler;
        CompileResult result = compiler.compile(*sourcePtr, filePath, *optionsPtr);
        if (!result.success) {
            return result;
        }

        // Disassemble the optimized module.
        const OptimizationResult optimized =
            SpirvDisassembler::optimize(result.spirv, optionsPtr->targetEnv, optionsPtr->optimizationLevel);
        if (optimized.ok) {
            result.spirvOptimized = optimized.words;
            if (!optimized.log.isEmpty()) {
                result.optimizerLog = optimized.log;
            }
            for (const Diagnostic& diag : optimized.diagnostics) {
                result.diagnostics.append(diag);
            }
        } else {
            result.spirvOptimized = result.spirv;
            for (const Diagnostic& diag : optimized.diagnostics) {
                result.diagnostics.append(diag);
            }
        }

        result.statsBeforeOpt = SpirvDisassembler::collectStatistics(result.spirv, optionsPtr->targetEnv);
        result.stats = SpirvDisassembler::collectStatistics(result.spirvOptimized, optionsPtr->targetEnv);
        result.versionString = SpirvDisassembler::moduleVersionString(result.spirvOptimized);

        const DisassemblyResult disassembly =
            SpirvDisassembler::disassemble(result.spirvOptimized, optionsPtr->targetEnv,
                                           optionsPtr->disasmFlags);
        if (disassembly.ok) {
            result.assembly = disassembly.text;
        } else {
            for (const Diagnostic& diag : disassembly.diagnostics) {
                result.diagnostics.append(diag);
            }
        }

        if (optionsPtr->validate) {
            const ValidationResult validation =
                SpirvDisassembler::validate(result.spirvOptimized, optionsPtr->targetEnv);
            for (const Diagnostic& diag : validation.diagnostics) {
                result.diagnostics.append(diag);
            }
        }

        const ReflectionResult reflection = SpirvReflection::reflect(result.spirvOptimized);
        if (reflection.ok) {
            result.reflection = reflection.data;
        }
        for (const Diagnostic& diag : reflection.diagnostics) {
            result.diagnostics.append(diag);
        }

        result.recountDiagnostics();
        return result;
    }));
}

void MainWindow::onCompile() {
    EditorTab* tab = currentTab();
    if (tab == nullptr) {
        return;
    }
    if (tab->source().trimmed().isEmpty()) {
        QMessageBox::information(this, tr("Compile"),
                                 tr("The current tab is empty.\n\nType some GLSL code first, "
                                    "or open one of the examples from examples/ on the left."));
        return;
    }
    startCompile(tab->filePath(), tab->source());
}

void MainWindow::onCompileAll() {
    if (m_tabs->count() == 0) {
        return;
    }
    // Compile every tab, then land on the first one with errors.
    int firstErrorTab = -1;
    for (int i = 0; i < m_tabs->count(); ++i) {
        auto* tab = qobject_cast<EditorTab*>(m_tabs->widget(i));
        if (tab == nullptr || tab->source().trimmed().isEmpty()) {
            continue;
        }
        m_tabs->setCurrentIndex(i);
        startCompile(tab->filePath(), tab->source());
        onCompileFinished();  // apply synchronously so the result reaches the tab
        if (!tab->lastResult().success && firstErrorTab < 0) {
            firstErrorTab = i;
        }
    }
    if (firstErrorTab >= 0) {
        m_tabs->setCurrentIndex(firstErrorTab);
    }
}

void MainWindow::onCompileFinished() {
    const quint64 generation = m_runningGeneration;
    m_runningGeneration = 0;

    const CompileResult result = m_watcher->result();
    const QString filePath = m_runningFile;
    m_runningFile.clear();

    if (generation != m_generation) {
        // A newer request superseded this one; drop the stale result.
        return;
    }

    // Attribute diagnostics with no file to the compiled tab.
    CompileResult attributed = result;
    for (Diagnostic& diag : attributed.diagnostics) {
        if (diag.file.isEmpty()) {
            diag.file = filePath;
        }
    }

    int index = findTab(filePath);
    if (index < 0) {
        index = m_tabs->currentIndex();
    }
    auto* tab = qobject_cast<EditorTab*>(m_tabs->widget(index));
    if (tab == nullptr) {
        return;
    }

    applyResult(filePath, attributed);

    if (index == m_tabs->currentIndex()) {
        // Show which tab was compiled when compiling all.
        m_statusLabel->setText(attributed.success ? tr("Compile succeeded")
                                                  : tr("Compile failed"));
    }
}

void MainWindow::applyResult(const QString& filePath, const CompileResult& result) {
    const int index = findTab(filePath);
    auto* tab = qobject_cast<EditorTab*>(m_tabs->widget(index >= 0 ? index : m_tabs->currentIndex()));

    CompileResult withHints = result;
    if (tab != nullptr && m_settings.syntaxHintsEnabled) {
        addStaticHints(tab, &withHints);
    }
    withHints.recountDiagnostics();

    if (tab != nullptr) {
        tab->setLastResult(withHints);
        if (withHints.reflection.has_value()) {
            tab->setReflectedNames(reflectedNames(*withHints.reflection));
        }
    }

    m_diagnostics = withHints.sortedDiagnostics();
    if (tab != nullptr) {
        tab->setDiagnostics(m_diagnostics);
    }
    m_diagnosticsPanel->setDiagnostics(m_diagnostics);

    // --- assembly panel -----------------------------------------------------
    m_asmPanel->setBinary(withHints.spirvOptimized);
    if (!withHints.success && withHints.assembly.isEmpty()) {
        QString message = withHints.failureTitle;
        if (!withHints.failureHint.isEmpty()) {
            message += QStringLiteral("\n\n") + withHints.failureHint;
        }
        m_asmPanel->setPlaceholder(message);
    } else {
        m_asmPanel->setAssembly(withHints.assembly, false);
        m_asmPanel->setStatistics(withHints.stats.wordCount, withHints.stats.instructionCount,
                                  withHints.stats.idBound, withHints.versionString);
    }

    // --- reflection ---------------------------------------------------------
    m_reflectPanel->setReflection(withHints.reflection.value_or(ReflectData{}),
                                  withHints.reflection.has_value());

    // --- log ----------------------------------------------------------------
    QStringList log;
    log << tr("=== Compile %1 ===").arg(filePath.isEmpty() ? tr("<untitled>") : filePath);
    log << tr("Elapsed %1 ms").arg(withHints.compileMs);
    log << tr("Success: %1").arg(withHints.success ? tr("yes") : tr("no"));
    log << tr("Errors %1 / Warnings %2 / Hints %3")
               .arg(withHints.errorCount)
               .arg(withHints.warningCount)
               .arg(withHints.hintCount);
    if (!withHints.generatorLog.isEmpty()) {
        log << QStringLiteral("\n--- glslang ---\n") + withHints.generatorLog;
    }
    if (!withHints.optimizerLog.isEmpty()) {
        log << QStringLiteral("\n--- spirv-opt ---\n") + withHints.optimizerLog;
    }
    if (!withHints.diagnostics.isEmpty()) {
        log << tr("\n--- Diagnostics ---");
        for (const Diagnostic& diag : withHints.diagnostics) {
            log << QStringLiteral("[%1] %2:%3 %4")
                       .arg(severityName(diag.severity))
                       .arg(diag.line)
                       .arg(diag.column)
                       .arg(diag.message);
        }
    }
    if (!withHints.failureTitle.isEmpty()) {
        log << QStringLiteral("\n%1\n%2").arg(withHints.failureTitle, withHints.failureHint);
    }
    m_outputPanel->setLog(log.join(QLatin1Char('\n')));
    m_outputPanel->setStatistics(withHints.stats, withHints.statsBeforeOpt,
                                 static_cast<OptimizationLevel>(
                                     m_optimizationCombo->currentData().toInt()),
                                 withHints.compileMs);
    m_outputPanel->setBinary(withHints.spirvOptimized);

    // --- status bar ---------------------------------------------------------
    m_timeLabel->setText(tr("Compile %1 ms").arg(withHints.compileMs));
    m_wordLabel->setText(tr("words %1").arg(withHints.statsBeforeOpt.wordCount));
    if (withHints.stats.wordCount != withHints.statsBeforeOpt.wordCount) {
        m_optimizedWordLabel->setText(
            tr("Optimized %1").arg(withHints.stats.wordCount));
    } else {
        m_optimizedWordLabel->clear();
    }
    m_saveSpvAction->setEnabled(withHints.success && !withHints.spirvOptimized.isEmpty());

    m_currentDiagnostic = m_diagnostics.isEmpty() ? -1 : 0;
    refreshTitles();

    if (!withHints.success && !withHints.failureTitle.isEmpty() && tab != nullptr) {
        // Surface the friendly failure in the problem list as well.
        m_statusLabel->setText(tr("%1 (%2 errors)")
                                   .arg(withHints.failureTitle)
                                   .arg(withHints.errorCount));
    }
}

QStringList MainWindow::reflectedNames(const ReflectData& data) const {
    QStringList names;
    for (const DescriptorBinding& binding : data.descriptors) {
        if (!binding.name.isEmpty()) {
            names << binding.name;
        }
        for (const ReflectMember& member : binding.members) {
            if (!member.name.isEmpty()) {
                names << member.name;
            }
        }
    }
    for (const PushConstantBlock& block : data.pushConstants) {
        if (!block.name.isEmpty()) {
            names << block.name;
        }
        for (const ReflectMember& member : block.members) {
            if (!member.name.isEmpty()) {
                names << member.name;
            }
        }
    }
    for (const StageVar& var : data.inputs) {
        if (!var.name.isEmpty() && !var.isBuiltin()) {
            names << var.name;
        }
    }
    for (const StageVar& var : data.outputs) {
        if (!var.name.isEmpty() && !var.isBuiltin()) {
            names << var.name;
        }
    }
    names.removeDuplicates();
    return names;
}

void MainWindow::onValidateRequested() {
    EditorTab* tab = currentTab();
    if (tab == nullptr || !tab->lastResult().success) {
        QMessageBox::information(this, tr("Validate"),
                                 tr("There is no module to validate yet.\n\nCompile "
                                    "successfully with F7 first."));
        return;
    }
    const ValidationResult validation =
        SpirvDisassembler::validate(tab->lastResult().spirvOptimized, m_settings.targetEnv);
    if (validation.ok) {
        QMessageBox::information(this, tr("Validate"),
                                 tr("SPIR-V validation passed (environment: %1).")
                                     .arg(targetEnvName(m_settings.targetEnv)));
        return;
    }
    QStringList messages;
    for (const Diagnostic& diag : validation.diagnostics) {
        messages << diag.message;
    }
    QMessageBox::warning(this, tr("Validation Failed"),
                         tr("SPIR-V validation failed:\n\n%1\n\nThe usual cause is a missing "
                            "layout(binding=) with automatic assignment turned off.")
                             .arg(messages.join(QLatin1Char('\n'))));
}

void MainWindow::onDisassembleRequested() {
    EditorTab* tab = currentTab();
    if (tab == nullptr || !tab->lastResult().success) {
        QMessageBox::information(this, tr("Disassemble"),
                                 tr("There is no module to disassemble yet.\n\nCompile "
                                    "successfully with F7 first."));
        return;
    }
    const DisassemblyResult disassembly =
        SpirvDisassembler::disassemble(tab->lastResult().spirvOptimized, m_settings.targetEnv,
                                       m_asmPanel->disasmFlags());
    if (!disassembly.ok) {
        QStringList messages;
        for (const Diagnostic& diag : disassembly.diagnostics) {
            messages << diag.message;
        }
        QMessageBox::warning(this, tr("Disassembly Failed"), messages.join(QLatin1Char('\n')));
        return;
    }
    m_asmPanel->setAssembly(disassembly.text, false);
}

void MainWindow::onDisasmFlagsChanged(unsigned flags) {
    m_settings.disasmFlags = flags;
    m_settings.save();
    // Re-disassemble in place when a module is already loaded.
    EditorTab* tab = currentTab();
    if (tab != nullptr && tab->lastResult().success) {
        const DisassemblyResult disassembly =
            SpirvDisassembler::disassemble(tab->lastResult().spirvOptimized, m_settings.targetEnv,
                                           flags);
        if (disassembly.ok) {
            m_asmPanel->setAssembly(disassembly.text, false);
        }
    }
}

void MainWindow::onOptimizationChanged() {
    m_settings.optimizationLevel =
        static_cast<OptimizationLevel>(m_optimizationCombo->currentData().toInt());
    m_settings.save();
    refreshStatus();
    // Optimization changes the module, so a recompile is required.
    EditorTab* tab = currentTab();
    if (tab != nullptr && !tab->isStale()) {
        tab->setStale(true);
        m_asmPanel->setAssembly(QString(), true);
    }
}

// --- diagnostics navigation --------------------------------------------------

void MainWindow::onDiagnosticActivated(const Diagnostic& diagnostic) {
    if (diagnostic.file.isEmpty() && diagnostic.line <= 0) {
        return;
    }
    int index = diagnostic.file.isEmpty() ? m_tabs->currentIndex() : findTab(diagnostic.file);
    if (index < 0) {
        // The diagnostic points at an #include; fall back to the current tab.
        index = m_tabs->currentIndex();
    }
    auto* tab = qobject_cast<EditorTab*>(m_tabs->widget(index));
    if (tab == nullptr) {
        return;
    }
    m_tabs->setCurrentIndex(index);
    tab->gotoLine(diagnostic.line, diagnostic.column);

    for (int i = 0; i < m_diagnostics.size(); ++i) {
        if (m_diagnostics.at(i).line == diagnostic.line &&
            m_diagnostics.at(i).message == diagnostic.message) {
            m_currentDiagnostic = i;
            break;
        }
    }
}

void MainWindow::onNextDiagnostic() {
    if (m_diagnostics.isEmpty()) {
        m_statusLabel->setText(tr("No problems"));
        return;
    }
    m_currentDiagnostic = (m_currentDiagnostic + 1) % m_diagnostics.size();
    onDiagnosticActivated(m_diagnostics.at(m_currentDiagnostic));
}

void MainWindow::onPreviousDiagnostic() {
    if (m_diagnostics.isEmpty()) {
        m_statusLabel->setText(tr("No problems"));
        return;
    }
    m_currentDiagnostic = (m_currentDiagnostic - 1 + m_diagnostics.size()) % m_diagnostics.size();
    onDiagnosticActivated(m_diagnostics.at(m_currentDiagnostic));
}

// --- tab / combo handling ----------------------------------------------------

void MainWindow::onTabChanged(int index) {
    Q_UNUSED(index);
    if (m_updatingTabs) {
        return;
    }
    EditorTab* tab = currentTab();
    if (tab == nullptr) {
        return;
    }

    // Reflect the tab's stage and version in the toolbar without re-triggering
    // the change handlers.
    const QSignalBlocker stageBlocker(m_stageCombo);
    const QSignalBlocker versionBlocker(m_versionCombo);
    m_stageCombo->setCurrentText(stageName(tab->stage()));
    m_versionCombo->setCurrentText(QString::number(m_settings.glslVersion));

    // Show this tab's cached products, marked stale when the source changed.
    const CompileResult& result = tab->lastResult();
    if (result.success && !tab->isStale()) {
        m_asmPanel->setAssembly(result.assembly, false);
        m_asmPanel->setStatistics(result.stats.wordCount, result.stats.instructionCount,
                                  result.stats.idBound, result.versionString);
        m_reflectPanel->setReflection(result.reflection.value_or(ReflectData{}),
                                      result.reflection.has_value());
        m_outputPanel->setBinary(result.spirvOptimized);
        m_diagnostics = result.sortedDiagnostics();
    } else {
        m_asmPanel->setAssembly(QString(), true);
        m_reflectPanel->setReflection(ReflectData{}, false);
        m_outputPanel->setBinary({});
        m_diagnostics.clear();
    }
    // Always refresh: the panel rebuilds cheaply and this avoids comparing a
    // QList against a QVector.
    m_diagnosticsPanel->setDiagnostics(m_diagnostics);

    m_positionLabel->setText(
        tr("Line %1, Column %2").arg(tab->currentLine()).arg(tab->currentColumn()));
    m_hintLabel->clear();
    refreshTitles();
}

void MainWindow::onEditorSelectionChanged(int index) {
    Q_UNUSED(index);
    refreshStatus();
}

void MainWindow::onStageComboChanged() {
    EditorTab* tab = currentTab();
    if (tab == nullptr) {
        return;
    }
    ShaderStage stage = tab->stage();
    if (stageFromKey(m_stageCombo->currentText(), &stage)) {
        tab->setStage(stage);
        tab->setStale(true);
        m_asmPanel->setAssembly(QString(), true);
    }
}

void MainWindow::onVersionComboChanged() {
    m_settings.glslVersion = m_versionCombo->currentData().toInt();
    m_settings.save();
    EditorTab* tab = currentTab();
    if (tab != nullptr) {
        tab->setStale(true);
        m_asmPanel->setAssembly(QString(), true);
    }
}

// --- settings / appearance ---------------------------------------------------

void MainWindow::onSettings() {
    SettingsDialog dialog(m_settings, this);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }
    m_settings = dialog.settings();
    m_settings.save();
    applyAppearance();

    // Push editor settings into every open tab.
    for (int i = 0; i < m_tabs->count(); ++i) {
        auto* tab = qobject_cast<EditorTab*>(m_tabs->widget(i));
        if (tab == nullptr) {
            continue;
        }
        tab->applyFont(m_settings.fontFamily, m_settings.fontSize);
        tab->editor()->setIndentSettings(m_settings.tabWidth, m_settings.softTabs);
        tab->completer()->setAutoTriggerEnabled(m_settings.completionEnabled);
        tab->setStale(true);
    }
    m_versionCombo->setCurrentText(QString::number(m_settings.glslVersion));
    m_vulkanCombo->setCurrentIndex(
        m_vulkanCombo->findData(static_cast<int>(m_settings.vulkanVersion)));
    refreshStatus();
}

void MainWindow::applyAppearance() {
    HighlightTheme theme;
    if (m_settings.theme == QLatin1String("dark")) {
        theme = HighlightTheme::dark();
    } else if (m_settings.theme == QLatin1String("light")) {
        theme = HighlightTheme::light();
    } else {
        // Follow the system palette: a dark window colour means a dark theme.
        const QColor window = palette().window().color();
        theme = window.lightness() < 128 ? HighlightTheme::dark() : HighlightTheme::light();
    }

    for (int i = 0; i < m_tabs->count(); ++i) {
        auto* tab = qobject_cast<EditorTab*>(m_tabs->widget(i));
        if (tab != nullptr) {
            tab->applyTheme(theme);
        }
    }
}

void MainWindow::onExportAssembly() {
    m_asmPanel->saveAs();
}

void MainWindow::onExportBinary() {
    m_asmPanel->saveBinaryAs();
}

void MainWindow::onCopyAssembly() {
    m_asmPanel->copyAll();
    m_statusLabel->setText(tr("Assembly copied to the clipboard"));
}

// --- examples ----------------------------------------------------------------

void MainWindow::loadExamples() {
    // Find the examples/ directory shipped next to the executable or in the source
    // tree, and list every supported file as a clickable link.
    const QStringList candidates = {
        QApplication::applicationDirPath() + QStringLiteral("/examples"),
        QApplication::applicationDirPath() + QStringLiteral("/../../examples"),
        QApplication::applicationDirPath() + QStringLiteral("/../../../examples"),
        QDir::currentPath() + QStringLiteral("/examples"),
    };

    QString directory;
    for (const QString& candidate : candidates) {
        if (QFileInfo::exists(candidate + QStringLiteral("/basic.vert"))) {
            directory = QDir(candidate).absolutePath();
            break;
        }
    }
    if (directory.isEmpty()) {
        m_exampleList->setPlainText(tr("(examples/ directory not found)"));
        return;
    }

    QStringList patterns;
    for (const QString& suffix : FileBrowser::supportedSuffixes()) {
        patterns << QStringLiteral("*.") + suffix;
    }
    const QStringList entries =
        QDir(directory).entryList(patterns, QDir::Files, QDir::Name);

    QStringList html;
    for (const QString& entry : entries) {
        const QString absolute = QDir(directory).absoluteFilePath(entry);
        html << QStringLiteral("<a href=\"%1\">%2</a>")
                    .arg(QUrl::fromLocalFile(absolute).toString(), entry.toHtmlEscaped());
    }
    m_exampleList->setHtml(html.join(QStringLiteral("<br>")));

    // Open the browser on the examples folder the first time, so a fresh install
    // shows something useful instead of an unrelated directory.
    if (m_fileBrowser != nullptr) {
        QSettings settings;
        const QString savedRoot = settings.value(QStringLiteral("files/browserRoot")).toString();
        if (savedRoot.isEmpty() || !QFileInfo::exists(savedRoot)) {
            m_fileBrowser->setRootPath(directory);
        }
    }
}

// --- heuristic syntax hints --------------------------------------------------

void MainWindow::addStaticHints(EditorTab* tab, CompileResult* result) const {
    if (tab == nullptr || result == nullptr) {
        return;
    }
    const QString source = tab->source();
    if (source.trimmed().isEmpty()) {
        return;
    }
    const QString fileName = tab->filePath();
    const auto add = [result, &fileName](Severity severity, int line, int column,
                                         const QString& message) {
        result->diagnostics.append(
            Diagnostic{severity, DiagSource::StaticHint, line, column, fileName, message});
    };

    const QStringList lines = source.split(QLatin1Char('\n'));

    // 1. #version must be the first meaningful line.
    {
        int versionLine = -1;
        for (int i = 0; i < lines.size(); ++i) {
            const QString trimmed = lines.at(i).trimmed();
            if (trimmed.isEmpty() || trimmed.startsWith(QLatin1String("//")) ||
                trimmed.startsWith(QLatin1String("/*"))) {
                continue;
            }
            if (trimmed.startsWith(QLatin1String("#version"))) {
                versionLine = i + 1;
            }
            break;  // first meaningful line decided
        }
        if (versionLine < 0) {
            add(Severity::Warning, 1, 1,
                tr("Missing #version directive. #version must be the first effective "
                   "statement in the file; write \"#version 450\"."));
        }
    }

    // 2. Brace and parenthesis balance.
    {
        int braces = 0;
        int parens = 0;
        int firstUnbalancedLine = 0;
        bool sawBrace = false;
        for (int i = 0; i < lines.size(); ++i) {
            const QString line = lines.at(i);
            // Strip line comments to avoid counting braces inside them.
            const int comment = line.indexOf(QLatin1String("//"));
            const QString code = comment >= 0 ? line.left(comment) : line;
            for (const QChar ch : code) {
                if (ch == QLatin1Char('{')) {
                    ++braces;
                    sawBrace = true;
                } else if (ch == QLatin1Char('}')) {
                    --braces;
                } else if (ch == QLatin1Char('(')) {
                    ++parens;
                } else if (ch == QLatin1Char(')')) {
                    --parens;
                }
            }
            if ((braces < 0 || parens < 0) && firstUnbalancedLine == 0) {
                firstUnbalancedLine = i + 1;
            }
        }
        if (firstUnbalancedLine > 0) {
            add(Severity::Error, firstUnbalancedLine, 1,
                tr("Unbalanced closing bracket: this '}' or ')' has no matching opener."));
        } else if (sawBrace && braces != 0) {
            add(Severity::Error, lines.size(), 1,
                tr("Unbalanced braces: %1 more '}' expected.").arg(braces));
        }
        if (parens > 0 && firstUnbalancedLine == 0) {
            add(Severity::Error, lines.size(), 1,
                tr("Unbalanced parentheses: %1 more ')' expected.").arg(parens));
        }
    }

    // 3. Missing semicolon.
    //
    // The rules live in core (hints::findMissingSemicolons) so they are covered by
    // tests/SpAll_hint_check without needing a GUI. A missing ';' is reported as an
    // error, not a hint: it is a hard compile error, and marking it red matches
    // what the compiler will say a moment later.
    for (const Diagnostic& diag : hints::findMissingSemicolons(source, fileName)) {
        result->diagnostics.append(diag);
    }


    // 4. Common spelling mistakes.
    //
    // QT_TR_NOOP marks the suggestions for extraction by lupdate without translating
    // them here; the tr() call inside the loop then looks each one up at runtime, so
    // these hints follow the UI language like every other string.
    struct Spelling {
        const char* pattern;
        const char* suggestion;
    };
    static const Spelling kSpellings[] = {
        {R"(\bvec4f\b)", QT_TR_NOOP("GLSL has no vec4f; use vec4")},
        {R"(\bvec3f\b)", QT_TR_NOOP("GLSL has no vec3f; use vec3")},
        {R"(\bvec2f\b)", QT_TR_NOOP("GLSL has no vec2f; use vec2")},
        {R"(\bfloat4\b)", QT_TR_NOOP("float4 is HLSL; use vec4 in GLSL")},
        {R"(\bfloat3\b)", QT_TR_NOOP("float3 is HLSL; use vec3 in GLSL")},
        {R"(\bfloat2\b)", QT_TR_NOOP("float2 is HLSL; use vec2 in GLSL")},
        {R"(\bmat3x3\b)", QT_TR_NOOP("mat3x3 should be written mat3")},
        {R"(\bmat4x4\b)", QT_TR_NOOP("mat4x4 should be written mat4")},
        {R"(\bmat2x2\b)", QT_TR_NOOP("mat2x2 should be written mat2")},
        {R"(\bsampler2d\b)", QT_TR_NOOP("sampler2d is the wrong case; use sampler2D")},
        {R"(\bSampler2D\b)", QT_TR_NOOP("Sampler2D is the wrong case; use sampler2D")},
        {R"(\btexture2D\s*\()",
         QT_TR_NOOP("Vulkan GLSL: use texture(...), not texture2D(...)")},
        {R"(\bgl_FragColor\b)",
         QT_TR_NOOP("gl_FragColor is compatibility-mode only; declare an out variable in Vulkan")},
    };
    for (const Spelling& spelling : kSpellings) {
        const QRegularExpression pattern(QString::fromLatin1(spelling.pattern));
        for (int i = 0; i < lines.size(); ++i) {
            const QRegularExpressionMatch match = pattern.match(lines.at(i));
            if (match.hasMatch()) {
                add(Severity::Hint, i + 1, match.capturedStart(0) + 1,
                    tr(spelling.suggestion));
            }
        }
    }

    // "#version 450 core" is redundant under Vulkan.
    for (int i = 0; i < lines.size(); ++i) {
        if (lines.at(i).trimmed().startsWith(QLatin1String("#version")) &&
            lines.at(i).contains(QLatin1String("core"))) {
            add(Severity::Hint, i + 1, 1,
                tr("Under a Vulkan target the profile keyword core is redundant; "
                   "write \"#version 450\" instead."));
        }
    }

    // 5. main() presence and gl_Position assignment.
    const bool hasMain = source.contains(QRegularExpression(QStringLiteral(R"(\bmain\s*\()")));
    if (!hasMain) {
        add(Severity::Error, 1, 1,
            tr("No main() function found. A shader must define void main()."));
    } else if (tab->stage() == ShaderStage::Vertex) {
        if (!source.contains(QLatin1String("gl_Position"))) {
            add(Severity::Warning, 1, 1,
                tr("The vertex shader never assigns gl_Position, so rasterization has "
                   "no valid position."));
        }
    }
}

}  // namespace spall
