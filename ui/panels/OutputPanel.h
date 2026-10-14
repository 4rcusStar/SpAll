// OutputPanel.h — binary dump / statistics / log tabs.
#pragma once

#include <QWidget>

#include "CompileResult.h"
#include "CompileOptions.h"

QT_BEGIN_NAMESPACE
class QPlainTextEdit;
class QTabWidget;
class QTreeWidget;
QT_END_NAMESPACE

namespace spall {

/// The right-hand "Stats" and "Log" tabs, plus a hex view of the module.
class OutputPanel : public QWidget {
    Q_OBJECT

public:
    explicit OutputPanel(QWidget* parent = nullptr);

    /// Instruction statistics for the statistics tab.
    void setStatistics(const SpirvStats& stats,
                       const SpirvStats& statsBeforeOptimization,
                       OptimizationLevel level,
                       qint64 compileMs);
    /// Log text assembled from compiler / validator / optimizer messages.
    void setLog(const QString& text);
    /// Hex dump of the optimized module.
    void setBinary(const QVector<quint32>& words);

    /// Which inner tab to raise (0 = stats, 1 = log, 2 = binary).
    void showTab(int index);

    /// Re-apply every user-visible string from the current catalogue.
    void retranslate();

private:
    QTabWidget* m_tabs = nullptr;
    QTreeWidget* m_statsTree = nullptr;
    QPlainTextEdit* m_logView = nullptr;
    QPlainTextEdit* m_binaryView = nullptr;
};

}  // namespace spall
