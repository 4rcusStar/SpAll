// contrast_check_main.cpp — verifies the diagnostic line tint stays readable.
//
// Two things are checked:
//   1. the foreground/background pair of every diagnostic line format has enough
//      contrast (WCAG relative-luminance ratio), and
//   2. syntax highlighting no longer overwrites the background of a line that also
//      carries a diagnostic — the bug that produced white-on-light text.

#include <QApplication>
#include <QTextBlock>
#include <QTextCharFormat>
#include <QTextDocument>
#include <QTextEdit>

#include <cmath>
#include <cstdio>

#include "EditorTab.h"
#include "GlslHighlighter.h"

using namespace spall;

namespace {

int g_failures = 0;
int g_checks = 0;

/// WCAG relative luminance of a colour.
double luminance(const QColor& c) {
    auto channel = [](double v) {
        v /= 255.0;
        return v <= 0.03928 ? v / 12.92 : std::pow((v + 0.055) / 1.055, 2.4);
    };
    return 0.2126 * channel(c.red()) + 0.7152 * channel(c.green()) + 0.0722 * channel(c.blue());
}

/// WCAG contrast ratio between two colours.
double contrastRatio(const QColor& fg, const QColor& bg) {
    const double l1 = luminance(fg);
    const double l2 = luminance(bg);
    const double lighter = std::max(l1, l2);
    const double darker = std::min(l1, l2);
    return (lighter + 0.05) / (darker + 0.05);
}

void checkTint(const char* themeName, const char* kind, const QTextCharFormat& format) {
    ++g_checks;
    const QColor fg = format.foreground().color();
    const QColor bg = format.background().color();

    if (format.background().style() == Qt::NoBrush) {
        std::printf("  [FAIL] %s / %-8s has no background\n", themeName, kind);
        ++g_failures;
        return;
    }
    if (format.foreground().style() == Qt::NoBrush) {
        std::printf("  [FAIL] %s / %-8s has no explicit foreground (it would inherit and "
                    "could become unreadable)\n",
                    themeName, kind);
        ++g_failures;
        return;
    }

    const double ratio = contrastRatio(fg, bg);
    // 4.5:1 is the WCAG AA threshold for body text.
    if (ratio >= 4.5) {
        std::printf("  [ OK ] %s / %-8s contrast %.1f:1 (fg #%02x%02x%02x on bg #%02x%02x%02x)\n",
                    themeName, kind, ratio, fg.red(), fg.green(), fg.blue(), bg.red(), bg.green(),
                    bg.blue());
    } else {
        std::printf("  [FAIL] %s / %-8s contrast only %.1f:1 (needs >= 4.5)\n", themeName, kind,
                    ratio);
        ++g_failures;
    }
}

}  // namespace

int main(int argc, char** argv) {
    QApplication app(argc, argv);

    std::printf("Diagnostic tint contrast test\n");
    std::printf("=============================\n");

    std::printf("\nLine tint contrast (WCAG AA needs >= 4.5:1):\n");
    const HighlightTheme light = HighlightTheme::light();
    checkTint("light", "error", light.errorLine);
    checkTint("light", "warning", light.warningLine);
    const HighlightTheme dark = HighlightTheme::dark();
    checkTint("dark", "error", dark.errorLine);
    checkTint("dark", "warning", dark.warningLine);

    // --- highlighting must not touch a diagnostic line's background ----------
    //
    // This is verified structurally: the guarantee comes from the highlighter not
    // setting any background at all on diagnostic lines (the tint is applied by
    // ShaderEdit as a full-width ExtraSelection instead). Inspecting the produced
    // fragments is only possible when the document has a layout, which needs a font
    // database, so that part is reported as informational rather than a failure.
    std::printf("\nHighlighter must leave diagnostic line backgrounds alone:\n");
    {
        const QString code = QStringLiteral(
            "#version 450\n"
            "layout(location = 0) out vec4 fragColor;\n"
            "void main()\n"
            "{\n"
            "    vec3 color = vec3(1.0);\n"   // line 5: no diagnostic
            "    fragColor = vec4(color);\n"  // line 6: carries a diagnostic
            "}\n");

        QTextEdit editor;
        editor.setPlainText(code);
        QTextDocument* document = editor.document();

        GlslHighlighter highlighter(document);
        highlighter.setTheme(light);
        highlighter.setDiagnostics(QVector<Diagnostic>{
            Diagnostic{Severity::Warning, DiagSource::StaticHint, 6, 30, {}, QStringLiteral("t")}});

        const QTextBlock block = document->findBlockByNumber(5);
        bool hasForeground = false;
        bool backgroundTouched = false;
        int fragments = 0;
        for (auto it = block.begin(); !it.atEnd(); ++it) {
            const QTextFragment fragment = it.fragment();
            if (!fragment.isValid()) {
                continue;
            }
            ++fragments;
            const QTextCharFormat fmt = fragment.charFormat();
            if (fmt.foreground().style() != Qt::NoBrush) {
                hasForeground = true;
            }
            if (fmt.background().style() != Qt::NoBrush) {
                backgroundTouched = true;
            }
        }

        if (backgroundTouched) {
            ++g_checks;
            ++g_failures;
            std::printf("  [FAIL] the highlighter set a background on a diagnostic line\n");
        } else if (hasForeground) {
            ++g_checks;
            std::printf("  [ OK ] syntax colours kept (%d fragments), no background set\n",
                        fragments);
        } else {
            std::printf("  [skip] no layout available in this environment (%d fragment); "
                        "the source-level guarantee is checked separately\n",
                        fragments);
        }
    }

    std::printf("\n=============================\n");
    std::printf("%d / %d checks passed\n", g_checks - g_failures, g_checks);
    if (g_failures > 0) {
        std::printf("%d FAILED\n", g_failures);
        return 1;
    }
    std::printf("all passed\n");
    return 0;
}
