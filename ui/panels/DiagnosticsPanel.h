// DiagnosticsPanel.h — the problem list with jump-to-source.
#pragma once

#include <QVector>
#include <QWidget>

#include "Diagnostic.h"

QT_BEGIN_NAMESPACE
class QCheckBox;
class QTreeWidget;
class QTreeWidgetItem;
QT_END_NAMESPACE

namespace spall {

class DiagnosticsPanel : public QWidget {
    Q_OBJECT

public:
    explicit DiagnosticsPanel(QWidget* parent = nullptr);

    /// Replace the list contents.
    void setDiagnostics(const QVector<Diagnostic>& diagnostics);
    void clear();

    /// Currently selected diagnostic, or a default-constructed one.
    Diagnostic selectedDiagnostic() const;
    /// All diagnostics, in display order.
    const QVector<Diagnostic>& diagnostics() const { return m_diagnostics; }

    /// Whether warnings/hints are shown.
    bool showWarnings() const;

    /// Re-apply every user-visible string from the current catalogue.
    void retranslate();

signals:
    /// A diagnostic was activated (double-click or Enter).
    void diagnosticActivated(const Diagnostic& diagnostic);

private slots:
    void onItemActivated(QTreeWidgetItem* item, int column);
    void applyFilter();

private:
    static QString severityGlyph(Severity severity);

    QTreeWidget* m_tree = nullptr;
    QCheckBox* m_showWarningsBox = nullptr;
    QVector<Diagnostic> m_diagnostics;
};

}  // namespace spall
