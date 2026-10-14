// ReflectPanel.h — reflection tree (descriptors, push constants, IO, spec consts).
#pragma once

#include <QString>
#include <QVector>
#include <QWidget>

#include "ReflectData.h"

QT_BEGIN_NAMESPACE
class QCheckBox;
class QLabel;
class QPushButton;
class QTreeWidget;
QT_END_NAMESPACE

namespace spall {

class ReflectPanel : public QWidget {
    Q_OBJECT

public:
    explicit ReflectPanel(QWidget* parent = nullptr);

    /// Replace the tree contents. An empty/!ok value shows the placeholder.
    void setReflection(const ReflectData& data, bool valid);

    /// The data currently displayed (used by copy/export).
    const ReflectData& data() const { return m_data; }

    /// Generate a std140-correct C++ struct for a UBO/SSBO block.
    /// Returns an empty string when the members cannot be expressed.
    static QString generateStruct(const QString& structName, const QVector<ReflectMember>& members);

    /// Round `value` up to the next multiple of `alignment`.
    static quint32 alignUp(quint32 value, quint32 alignment);

    /// Re-apply every user-visible string from the current catalogue.
    void retranslate();

signals:
    /// Emitted when the "hide unused" setting changes.
    void hideUnusedChanged(bool hide);

private slots:
    void copyAsText();
    void generateStructForSelection();

private:
    void rebuild();

    QTreeWidget* m_tree = nullptr;
    QCheckBox* m_hideUnusedBox = nullptr;
    QLabel* m_summaryLabel = nullptr;
    QPushButton* m_copyButton = nullptr;
    QPushButton* m_structButton = nullptr;

    ReflectData m_data;
    bool m_valid = false;
};

}  // namespace spall
