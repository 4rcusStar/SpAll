// hint_check_main.cpp — regression test for the pre-compile syntax hints.
//
// Exercises core's hints::findMissingSemicolons against the false positives that
// were reported from real shader code, plus the true positives it must still
// catch. The checker runs before compilation, so a wrong hint is visible while
// typing; that makes both directions worth locking down.

#include <QCoreApplication>
#include <QString>
#include <QStringList>
#include <QVector>

#include <cstdio>

#include "Diagnostic.h"

using namespace spall;

namespace {

int g_failures = 0;
int g_checks = 0;

/// Join lines into one source text the way a file would look.
QString source(const QStringList& lines) {
    return lines.join(QLatin1Char('\n')) + QLatin1Char('\n');
}

/// The hint checker must produce no findings for this snippet.
void expectClean(const char* name, const QStringList& lines) {
    ++g_checks;
    const QVector<Diagnostic> found = hints::findMissingSemicolons(source(lines), QStringLiteral("t.glsl"));
    if (found.isEmpty()) {
        std::printf("  [ OK ] %-38s no finding\n", name);
        return;
    }
    ++g_failures;
    std::printf("  [FAIL] %-38s %d finding(s):\n", name, static_cast<int>(found.size()));
    for (const Diagnostic& d : found) {
        std::printf("           line %d: %s\n", d.line, d.message.toUtf8().constData());
    }
}

/// The hint checker must flag exactly `expectLine`, and report it as an Error.
void expectFinding(const char* name, const QStringList& lines, int expectLine) {
    ++g_checks;
    const QVector<Diagnostic> found =
        hints::findMissingSemicolons(source(lines), QStringLiteral("t.glsl"));

    bool hit = false;
    bool severityOk = true;
    for (const Diagnostic& d : found) {
        if (d.line == expectLine) {
            hit = true;
            severityOk = (d.severity == Severity::Error);
        }
    }
    if (hit && severityOk) {
        std::printf("  [ OK ] %-38s flagged line %d as Error\n", name, expectLine);
        return;
    }
    ++g_failures;
    if (hit) {
        std::printf("  [FAIL] %-38s line %d flagged but not as Error\n", name, expectLine);
    } else {
        std::printf("  [FAIL] %-38s expected line %d, got %d finding(s)\n", name, expectLine,
                    static_cast<int>(found.size()));
        for (const Diagnostic& d : found) {
            std::printf("           line %d\n", d.line);
        }
    }
}

}  // namespace

int main(int argc, char** argv) {
    QCoreApplication app(argc, argv);

    std::printf("Pre-compile hint regression test\n");
    std::printf("================================\n");

    // --- the exact false positives reported from real shader code ------------
    std::printf("\nReported false positives (must stay clean):\n");

    expectClean("if with trailing // comment",
                QStringList{QStringLiteral("void main()"), QStringLiteral("{"),
                            QStringLiteral("    if ( c.r <= 0.0031308 )// wrongly highlighted"),
                            QStringLiteral("    {"),
                            QStringLiteral("        result.r = c.r * 12.92;"),
                            QStringLiteral("    } else "), QStringLiteral("    {"),
                            QStringLiteral("        result.r = 1.055 * pow( c.r, 1.0 / 2.4 ) - 0.055;"),
                            QStringLiteral("    }"), QStringLiteral("}")});

    expectClean("mat3( with continuation lines",
                QStringList{QStringLiteral("void main()"), QStringLiteral("{"),
                            QStringLiteral("    mat3 TBN;"),
                            QStringLiteral("    TBN = mat3(// wrongly highlighted"),
                            QStringLiteral("        tangent,"),
                            QStringLiteral("        bitangent,"),
                            QStringLiteral("        normalize( vNormal )// wrongly highlighted"),
                            QStringLiteral("    );"), QStringLiteral("}")});

    expectClean("else on its own line",
                QStringList{QStringLiteral("void main()"), QStringLiteral("{"),
                            QStringLiteral("    if (a) { b(); }"),
                            QStringLiteral("    else"),
                            QStringLiteral("    {"), QStringLiteral("        c();"),
                            QStringLiteral("    }"), QStringLiteral("}")});

    expectClean("multi-line call arguments",
                QStringList{QStringLiteral("void main()"), QStringLiteral("{"),
                            QStringLiteral("    float x = compute("), QStringLiteral("        a,"),
                            QStringLiteral("        b);"), QStringLiteral("}")});

    expectClean("trailing binary operator",
                QStringList{QStringLiteral("void main()"), QStringLiteral("{"),
                            QStringLiteral("    float y = a +"), QStringLiteral("        b;"),
                            QStringLiteral("}")});

    expectClean("if without braces",
                QStringList{QStringLiteral("void main()"), QStringLiteral("{"),
                            QStringLiteral("    if (c.r <= 0.0031308)"),
                            QStringLiteral("        result.r = c.r * 12.92;"),
                            QStringLiteral("}")});

    expectClean("commented-out code",
                QStringList{QStringLiteral("void main()"), QStringLiteral("{"),
                            QStringLiteral("    // result.r = c.r * 12.92"),
                            QStringLiteral("    /* result.g = c.g * 12.92"),
                            QStringLiteral("       result.b = c.b * 12.92 */"),
                            QStringLiteral("    done();"), QStringLiteral("}")});

    expectClean("block comment spanning lines",
                QStringList{QStringLiteral("void main()"), QStringLiteral("{"),
                            QStringLiteral("    /* note"), QStringLiteral("       more"),
                            QStringLiteral("       end */"), QStringLiteral("    done();"),
                            QStringLiteral("}")});

    expectClean("globals outside any body",
                QStringList{QStringLiteral("#version 450"),
                            QStringLiteral("layout(location = 0) out vec4 fragColor"),
                            QStringLiteral("void main()"), QStringLiteral("{"),
                            QStringLiteral("    fragColor = vec4(1.0);"), QStringLiteral("}")});

    expectClean("control flow forms",
                QStringList{QStringLiteral("void main()"), QStringLiteral("{"),
                            QStringLiteral("    for (int i = 0; i < 4; ++i)"),
                            QStringLiteral("    {"), QStringLiteral("        sum += i;"),
                            QStringLiteral("    }"), QStringLiteral("    while (ready)"),
                            QStringLiteral("    {"), QStringLiteral("        step();"),
                            QStringLiteral("    }"), QStringLiteral("    do"),
                            QStringLiteral("    {"), QStringLiteral("        tick();"),
                            QStringLiteral("    } while (running);"),
                            QStringLiteral("    switch (mode)"), QStringLiteral("    {"),
                            QStringLiteral("        default:"), QStringLiteral("            break;"),
                            QStringLiteral("    }"), QStringLiteral("}")});

    expectClean("struct and function definitions",
                QStringList{QStringLiteral("struct Material"), QStringLiteral("{"),
                            QStringLiteral("    vec4 tint;"), QStringLiteral("};"),
                            QStringLiteral("float f()"), QStringLiteral("{"),
                            QStringLiteral("    return 1.0;"), QStringLiteral("}")});

    // --- true positives that must still be caught ---------------------------
    std::printf("\nTrue positives (must still be flagged as Error):\n");

    expectFinding("missing ; after declaration",
                  QStringList{QStringLiteral("void main()"), QStringLiteral("{"),
                              QStringLiteral("    vec3 color = vec3(1.0)"),
                              QStringLiteral("    fragColor = vec4(color, 1.0);"),
                              QStringLiteral("}")},
                  3);

    expectFinding("missing ; after assignment",
                  QStringList{QStringLiteral("void main()"), QStringLiteral("{"),
                              QStringLiteral("    x = 1.0"), QStringLiteral("    y = 2.0;"),
                              QStringLiteral("}")},
                  3);

    expectFinding("missing ; after call statement",
                  QStringList{QStringLiteral("void main()"), QStringLiteral("{"),
                              QStringLiteral("    doWork(1.0)"), QStringLiteral("}")},
                  3);

    expectFinding("missing ; after return value",
                  QStringList{QStringLiteral("float f()"), QStringLiteral("{"),
                              QStringLiteral("    return 1.0"), QStringLiteral("}")},
                  3);

    expectFinding("missing ; before a trailing comment",
                  QStringList{QStringLiteral("void main()"), QStringLiteral("{"),
                              QStringLiteral("    x = 1.0   // forgot the semicolon"),
                              QStringLiteral("}")},
                  3);

    std::printf("\n================================\n");
    std::printf("%d / %d checks passed\n", g_checks - g_failures, g_checks);
    if (g_failures > 0) {
        std::printf("%d FAILED\n", g_failures);
        return 1;
    }
    std::printf("all passed\n");
    return 0;
}
