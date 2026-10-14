// DiagnosticsPanel.cpp — problem list rendering and filtering.
#include "DiagnosticsPanel.h"

#include <QCheckBox>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QTreeWidget>
#include <QVBoxLayout>

namespace spall {

DiagnosticsPanel::DiagnosticsPanel(QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(4, 4, 4, 4);
    layout->setSpacing(4);

    auto* header = new QHBoxLayout();
    m_showWarningsBox = new QCheckBox(tr("Show warnings and hints"), this);
    m_showWarningsBox->setChecked(true);
    header->addWidget(m_showWarningsBox);
    header->addStretch(1);
    layout->addLayout(header);

    m_tree = new QTreeWidget(this);
    m_tree->setColumnCount(4);
    m_tree->setHeaderLabels({tr("Severity"), tr("Category"),
                             tr("Location"), tr("Message")});
    m_tree->setRootIsDecorated(false);
    m_tree->setAlternatingRowColors(true);
    m_tree->setUniformRowHeights(true);
    m_tree->header()->setSectionResizeMode(3, QHeaderView::Stretch);
    m_tree->setColumnWidth(0, 70);
    m_tree->setColumnWidth(1, 100);
    m_tree->setColumnWidth(2, 200);
    layout->addWidget(m_tree, 1);

    connect(m_tree, &QTreeWidget::itemActivated, this, &DiagnosticsPanel::onItemActivated);
    connect(m_showWarningsBox, &QCheckBox::toggled, this, &DiagnosticsPanel::applyFilter);
}

void DiagnosticsPanel::retranslate() {
    m_showWarningsBox->setText(tr("Show warnings and hints"));
    m_tree->setHeaderLabels({tr("Severity"), tr("Category"),
                             tr("Location"), tr("Message")});

    // The rows carry translated text too, so they are regenerated from m_diagnostics.
    applyFilter();
}

QString DiagnosticsPanel::severityGlyph(Severity severity) {
    switch (severity) {
        case Severity::Error:   return tr("✖ Error");
        case Severity::Warning: return tr("⚠ Warning");
        case Severity::Info:    return tr("ℹ Info");
        case Severity::Hint:    return tr("· Hint");
    }
    return tr("· Hint");
}

void DiagnosticsPanel::setDiagnostics(const QVector<Diagnostic>& diagnostics) {
    m_diagnostics = diagnostics;
    applyFilter();
}

void DiagnosticsPanel::clear() {
    m_diagnostics.clear();
    m_tree->clear();
}

bool DiagnosticsPanel::showWarnings() const {
    return m_showWarningsBox->isChecked();
}

void DiagnosticsPanel::applyFilter() {
    m_tree->clear();
    const bool showNonErrors = m_showWarningsBox->isChecked();

    for (int i = 0; i < m_diagnostics.size(); ++i) {
        const Diagnostic& diag = m_diagnostics.at(i);
        if (!showNonErrors && diag.severity != Severity::Error) {
            continue;
        }

        QString location;
        if (diag.hasPosition()) {
            const QString file = diag.file.isEmpty() ? tr("<editor>")
                                                     : QFileInfo(diag.file).fileName();
            location = diag.column > 0
                           ? QStringLiteral("%1:%2:%3").arg(file).arg(diag.line).arg(diag.column)
                           : QStringLiteral("%1:%2").arg(file).arg(diag.line);
        } else {
            location = tr("Global");
        }

        auto* item = new QTreeWidgetItem(m_tree);
        item->setText(0, severityGlyph(diag.severity));
        item->setText(1, diagSourceName(diag.source));
        item->setText(2, location);
        item->setText(3, diag.message);
        item->setData(0, Qt::UserRole, i);  // index into m_diagnostics
        item->setToolTip(3, diag.message);

        switch (diag.severity) {
            case Severity::Error:   item->setForeground(0, QColor(0xc0, 0x28, 0x28)); break;
            case Severity::Warning: item->setForeground(0, QColor(0xb0, 0x7a, 0x00)); break;
            default:                item->setForeground(0, QColor(0x60, 0x60, 0x60)); break;
        }
    }

    // The list is already severity-sorted by the caller (sortedDiagnostics()).
    for (int c = 0; c < 3; ++c) {
        m_tree->resizeColumnToContents(c);
    }
}

Diagnostic DiagnosticsPanel::selectedDiagnostic() const {
    const QList<QTreeWidgetItem*> selected = m_tree->selectedItems();
    if (selected.isEmpty()) {
        return {};
    }
    const int index = selected.first()->data(0, Qt::UserRole).toInt();
    if (index < 0 || index >= m_diagnostics.size()) {
        return {};
    }
    return m_diagnostics.at(index);
}

void DiagnosticsPanel::onItemActivated(QTreeWidgetItem* item, int column) {
    Q_UNUSED(column);
    if (item == nullptr) {
        return;
    }
    const int index = item->data(0, Qt::UserRole).toInt();
    if (index < 0 || index >= m_diagnostics.size()) {
        return;
    }
    emit diagnosticActivated(m_diagnostics.at(index));
}

}  // namespace spall
