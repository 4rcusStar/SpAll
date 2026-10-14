// LineNumberArea.cpp — line-number gutter painting.
#include "LineNumberArea.h"

#include <QPainter>
#include <QPlainTextEdit>
#include <QTextBlock>

#include "EditorTab.h"

namespace spall {

LineNumberArea::LineNumberArea(QPlainTextEdit* editor) : QWidget(editor), m_editor(editor) {}

QSize LineNumberArea::sizeHint() const {
    if (auto* shaderEdit = qobject_cast<ShaderEdit*>(m_editor)) {
        return QSize(shaderEdit->lineNumberAreaWidth(), 0);
    }
    return QSize(40, 0);
}

void LineNumberArea::paintEvent(QPaintEvent* event) {
    if (auto* shaderEdit = qobject_cast<ShaderEdit*>(m_editor)) {
        shaderEdit->paintLineNumbers(event);
    }
}

}  // namespace spall
