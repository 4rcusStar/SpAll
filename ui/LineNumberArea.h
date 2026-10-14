// LineNumberArea.h — gutter widget for EditorTab's line numbers.
#pragma once

#include <QWidget>

QT_BEGIN_NAMESPACE
class QPlainTextEdit;
QT_END_NAMESPACE

namespace spall {

/// Paints the line-number gutter for a QPlainTextEdit. EditorTab owns the
/// sizing and update logic; this widget only draws.
class LineNumberArea : public QWidget {
    Q_OBJECT

public:
    explicit LineNumberArea(QPlainTextEdit* editor);

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QPlainTextEdit* m_editor = nullptr;
};

}  // namespace spall
