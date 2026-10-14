// MainWindow.h — application shell: toolbar, docks, tabs and compile orchestration.
//
// All compilation runs on a worker thread through QtConcurrent. Only one job is
// in flight at a time; starting a new one invalidates the previous job's result
// via a generation counter so stale results can never overwrite fresh UI state.
#pragma once

#include <QFutureWatcher>
#include <QMainWindow>
#include <QVector>

#include "CompileOptions.h"
#include "CompileResult.h"
#include "Diagnostic.h"
#include "SettingsDialog.h"

QT_BEGIN_NAMESPACE
class QAction;
class QActionGroup;
class QComboBox;
class QDockWidget;
class QEvent;
class QLabel;
class QMenu;
class QPlainTextEdit;
class QTabWidget;
class QTextBrowser;
class QToolBar;
QT_END_NAMESPACE

namespace spall {

class AsmPanel;
class DiagnosticsPanel;
class EditorTab;
class FileBrowser;
class OutputPanel;
class ReflectPanel;
class Translator;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent* event) override;
    /// Reacts to QEvent::LanguageChange by re-applying every visible string.
    void changeEvent(QEvent* event) override;

private slots:
    // --- file actions -------------------------------------------------------
    void onNewFile();
    void onOpenFile();
    /// Open `path` in a tab, reusing an existing tab when already open.
    void openFilePath(const QString& path);
    void onSave();
    void onSaveAs();
    void onReload();
    void onOpenRecent();
    void onCloseTab(int index);

    // --- compile ------------------------------------------------------------
    void onCompile();
    void onCompileAll();
    void onCompileFinished();
    void onValidateRequested();
    void onDisassembleRequested();
    void onDisasmFlagsChanged(unsigned flags);
    void onOptimizationChanged();

    // --- navigation ---------------------------------------------------------
    void onDiagnosticActivated(const Diagnostic& diagnostic);
    void onNextDiagnostic();
    void onPreviousDiagnostic();

    // --- view / settings ----------------------------------------------------
    void onTabChanged(int index);
    void onEditorSelectionChanged(int index);
    void onStageComboChanged();
    void onVersionComboChanged();
    void onSettings();
    void onAbout();
    void onExportAssembly();
    void onExportBinary();
    void onCopyAssembly();
    /// Switch the UI language; `code` is a Translator language code.
    void onLanguageSelected(const QString& code);

private:
    void buildActions();
    void buildToolbar();
    void buildDocks();
    void buildStatusBar();
    /// Create the menu bar (including the Language menu). Called once from the
    /// constructor; retranslate() presupposes these menus exist.
    void buildMenuBar();
    void restoreLayout();
    void saveLayout();

    /// The tab the user is currently editing, or nullptr.
    EditorTab* currentTab() const;
    /// Index of the tab holding `filePath`, or -1.
    int findTab(const QString& filePath) const;

    /// Snapshot of one tab, captured on the GUI thread for a worker.
    struct CompileJob {
        QString filePath;
        QString source;
        CompileOptions options;
        quint64 generation = 0;
    };

    /// Build the CompileOptions for a tab from the current UI state.
    CompileOptions optionsForTab(const EditorTab* tab) const;
    /// Kick off a background compile. No-op when one is already running.
    void startCompile(const QString& filePath, const QString& source);
    /// Apply a finished result to its tab and all panels.
    void applyResult(const QString& filePath, const CompileResult& result);
    /// Run the cheap heuristic hints and add them to the result.
    void addStaticHints(EditorTab* tab, CompileResult* result) const;
    /// Update the window title, tab texts and status bar.
    void refreshTitles();
    void refreshStatus();
    /// Enable/disable actions that need a document.
    void updateActionState();
    /// Collect reflected symbol names for the completer.
    QStringList reflectedNames(const ReflectData& data) const;

    void addRecentFile(const QString& path);
    void rebuildRecentMenu();
    /// Ask about unsaved changes; false means "cancel the operation".
    bool maybeSaveTab(EditorTab* tab);
    void applyAppearance();
    void loadExamples();

    /// Build the Language menu from Translator::available().
    void buildLanguageMenu();
    /// Re-apply every user-visible string from the current catalogue. Called on
    /// QEvent::LanguageChange, so switching language needs no restart.
    void retranslate();

    /// Re-apply the strings this window owns *and* ask its panels to do the same.
    ///
    /// Qt's LanguageChange event does not reach widgets unless they are top-level, so
    /// the docked panels (which are children) never receive one on their own. Every
    /// retranslate() calls this so the whole window updates in one pass.
    void triggerRetranslate();

public:
    /// Headless self-check used by `SpAll --selftest`. It types into the current
    /// tab and reports whether the completion popup appeared, so the real window
    /// (not a test harness widget) can be verified automatically. Returns 0 on
    /// success, non-zero on the first failed expectation.
    int runCompletionSelfTest();

    /// Force the UI language for this run, overriding the persisted setting.
    /// Set from `--lang=<code>` before the window is constructed.
    static void setLanguageOverride(const QString& code);

private:
    // --- widgets ------------------------------------------------------------
    QTabWidget* m_tabs = nullptr;
    FileBrowser* m_fileBrowser = nullptr;
    QTextBrowser* m_exampleList = nullptr;  ///< clickable list of bundled examples
    AsmPanel* m_asmPanel = nullptr;
    ReflectPanel* m_reflectPanel = nullptr;
    OutputPanel* m_outputPanel = nullptr;
    DiagnosticsPanel* m_diagnosticsPanel = nullptr;
    QDockWidget* m_leftDock = nullptr;
    QDockWidget* m_rightDock = nullptr;
    QDockWidget* m_bottomDock = nullptr;
    QTabWidget* m_rightTabs = nullptr;

    QComboBox* m_stageCombo = nullptr;
    QComboBox* m_versionCombo = nullptr;
    QComboBox* m_vulkanCombo = nullptr;
    QComboBox* m_optimizationCombo = nullptr;
    QAction* m_compileAction = nullptr;
    QAction* m_compileAllAction = nullptr;
    QAction* m_validateAction = nullptr;
    QAction* m_disasmAction = nullptr;
    QAction* m_saveAction = nullptr;
    QAction* m_saveAsAction = nullptr;
    QAction* m_reloadAction = nullptr;
    QAction* m_saveSpvAction = nullptr;
    QAction* m_newAction = nullptr;
    QAction* m_openAction = nullptr;
    QAction* m_exportAssemblyAction = nullptr;
    QAction* m_copyAssemblyAction = nullptr;
    QAction* m_settingsAction = nullptr;
    QAction* m_aboutAction = nullptr;

    // Language menu. The menus and the toolbar's separator labels are kept because
    // retranslate() has to reach them; the menu bar is built lazily, so they cannot be
    // looked up by walking the widget tree.
    QMenu* m_fileMenu = nullptr;
    QMenu* m_recentMenu = nullptr;
    QMenu* m_buildMenu = nullptr;
    QMenu* m_viewMenu = nullptr;
    QMenu* m_helpMenu = nullptr;
    QMenu* m_languageMenu = nullptr;
    QActionGroup* m_languageGroup = nullptr;
    QToolBar* m_toolBar = nullptr;
    QLabel* m_stageLabel = nullptr;
    QLabel* m_versionLabel = nullptr;
    QLabel* m_vulkanLabel = nullptr;
    QLabel* m_optimizationLabel = nullptr;
    Translator* m_translator = nullptr;

    QLabel* m_positionLabel = nullptr;
    QLabel* m_encodingLabel = nullptr;
    QLabel* m_timeLabel = nullptr;
    QLabel* m_wordLabel = nullptr;
    QLabel* m_optimizedWordLabel = nullptr;
    QLabel* m_statusLabel = nullptr;
    QLabel* m_hintLabel = nullptr;

    // --- state --------------------------------------------------------------
    AppSettings m_settings;
    QVector<QString> m_recentFiles;

    /// Generation counter: results tagged with an older value are discarded.
    quint64 m_generation = 0;
    /// Generation of the job currently running (0 when idle).
    quint64 m_runningGeneration = 0;
    QString m_runningFile;

    QFutureWatcher<CompileResult>* m_watcher = nullptr;
    std::shared_ptr<QString> m_pendingSource;
    std::shared_ptr<CompileOptions> m_pendingOptions;

    /// Diagnostics of the most recent compile, in display order.
    QVector<Diagnostic> m_diagnostics;
    int m_currentDiagnostic = -1;

    /// Guards refreshTitles() against re-entrancy while tabs are rebuilt.
    bool m_updatingTabs = false;
};

}  // namespace spall
