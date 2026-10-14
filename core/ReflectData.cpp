// ReflectData.cpp — plain-text rendering of reflection results.
#include "ReflectData.h"

#include <QCoreApplication>
#include <QStringList>

namespace spall {

namespace {

// Localised string for this translation context.
//
// A macro rather than a function on purpose: lupdate parses sources statically
// and cannot see through a call that passes the text on to
// QCoreApplication::translate(). Spelling the lookup out here means every trs(QT_TRANSLATE_NOOP("Reflection", ...))
// call site is extracted automatically, while the runtime behaviour is unchanged.
/// Localised string for this translation context.
///
/// The text is passed through QT_TRANSLATE_NOOP at every call site, which is what
/// lets lupdate extract it statically; this function then performs the runtime
/// lookup. core/ has no QObject, so tr() is not available there.
QString trs(const char* text) {
    return QCoreApplication::translate("Reflection", text);
}

/// Pad `text` with spaces to `width` for the aligned text tables.
QString pad(const QString& text, int width) {
    if (text.size() >= width) {
        return text;
    }
    return text + QString(width - text.size(), QLatin1Char(' '));
}

QString heading(const QString& title) {
    return QStringLiteral("\n== %1 ==\n").arg(title);
}

/// Recursively render block members with indentation.
void renderMembers(const QVector<ReflectMember>& members, int depth, QStringList* lines) {
    const QString indent(depth * 2, QLatin1Char(' '));
    for (const ReflectMember& member : members) {
        lines->append(QStringLiteral("  %1%2  %3  offset=%4  size=%5  padded=%6")
                          .arg(indent, pad(member.name, qMax(4, 24 - depth * 2)), pad(member.typeName, 18))
                          .arg(member.offset)
                          .arg(member.size)
                          .arg(member.paddedSize));
        if (!member.members.isEmpty()) {
            renderMembers(member.members, depth + 1, lines);
        }
    }
}

}  // namespace

QString ReflectData::toPlainText() const {
    QStringList lines;

    // --- execution ----------------------------------------------------------
    lines << QStringLiteral("SpAll reflection");
    lines << QStringLiteral("entry point    : %1").arg(execution.entryPoint);
    lines << QStringLiteral("execution model: %1").arg(execution.executionModel);
    lines << QStringLiteral("generator      : %1").arg(execution.generator);
    if (execution.localSizeX > 0 || execution.localSizeY > 0 || execution.localSizeZ > 0) {
        lines << QStringLiteral("local size     : %1 x %2 x %3")
                     .arg(execution.localSizeX)
                     .arg(execution.localSizeY)
                     .arg(execution.localSizeZ);
    }
    if (execution.invocations > 0) {
        lines << QStringLiteral("invocations    : %1").arg(execution.invocations);
    }
    if (execution.outputVertices > 0) {
        lines << QStringLiteral("output vertices: %1").arg(execution.outputVertices);
    }
    if (!execution.executionModes.isEmpty()) {
        lines << QStringLiteral("execution modes: %1").arg(execution.executionModes.join(QStringLiteral(", ")));
    }

    // --- descriptor bindings -------------------------------------------------
    if (!descriptors.isEmpty()) {
        lines << heading(QStringLiteral("Descriptor Bindings"));
        lines << QStringLiteral("  %1 %2 %3 %4 %5 %6 %7")
                     .arg(pad(QStringLiteral("set"), 4),
                          pad(QStringLiteral("bind"), 5),
                          pad(QStringLiteral("type"), 24),
                          pad(QStringLiteral("name"), 24),
                          pad(QStringLiteral("array"), 6),
                          pad(QStringLiteral("size"), 8),
                          QStringLiteral("used"));
        for (const DescriptorBinding& binding : descriptors) {
            lines << QStringLiteral("  %1 %2 %3 %4 %5 %6 %7")
                         .arg(pad(QString::number(binding.set), 4),
                              pad(QString::number(binding.binding), 5),
                              pad(binding.typeName, 24),
                              pad(binding.name, 24),
                              pad(QString::number(binding.count), 6),
                              pad(QString::number(binding.size), 8),
                              binding.used ? QStringLiteral("yes") : QStringLiteral("no"));
            renderMembers(binding.members, 2, &lines);
        }
    }

    // --- push constants ------------------------------------------------------
    if (!pushConstants.isEmpty()) {
        lines << heading(QStringLiteral("Push Constants"));
        for (const PushConstantBlock& block : pushConstants) {
            lines << QStringLiteral("  block %1  offset=%2  size=%3")
                         .arg(block.name)
                         .arg(block.offset)
                         .arg(block.size);
            renderMembers(block.members, 3, &lines);
        }
    }

    // --- stage inputs / outputs ---------------------------------------------
    const auto renderStageVars = [&lines](const QString& title, const QVector<StageVar>& vars) {
        if (vars.isEmpty()) {
            return;
        }
        lines << heading(title);
        lines << QStringLiteral("  %1 %2 %3 %4 %5")
                     .arg(pad(QStringLiteral("loc"), 5),
                          pad(QStringLiteral("comp"), 5),
                          pad(QStringLiteral("name"), 22),
                          pad(QStringLiteral("type"), 16),
                          QStringLiteral("format / builtin"));
        for (const StageVar& var : vars) {
            QString extra = var.format;
            if (var.isBuiltin()) {
                extra = QStringLiteral("%1 (builtin %2)").arg(var.builtinName, QString::number(var.builtin));
            }
            lines << QStringLiteral("  %1 %2 %3 %4 %5")
                         .arg(pad(QString::number(var.location), 5),
                              pad(QString::number(var.component), 5),
                              pad(var.name, 22),
                              pad(var.typeName, 16),
                              extra);
        }
    };
    renderStageVars(QStringLiteral("Stage Inputs"), inputs);
    renderStageVars(QStringLiteral("Stage Outputs"), outputs);

    // --- specialization constants -------------------------------------------
    if (!specConstants.isEmpty()) {
        lines << heading(QStringLiteral("Specialization Constants"));
        lines << QStringLiteral("  %1 %2 %3")
                     .arg(pad(QStringLiteral("id"), 6),
                          pad(QStringLiteral("name"), 24),
                          QStringLiteral("default"));
        for (const SpecConstant& constant : specConstants) {
            lines << QStringLiteral("  %1 %2 %3")
                         .arg(pad(QString::number(constant.constantId), 6),
                              pad(constant.name, 24),
                              constant.defaultValue);
        }
    }

    if (descriptors.isEmpty() && pushConstants.isEmpty() && inputs.isEmpty() && outputs.isEmpty() &&
        specConstants.isEmpty()) {
        lines << trs(QT_TRANSLATE_NOOP("Reflection", "\n(this module exposes no descriptor, push constant or IO variable)"));
    }

    return lines.join(QLatin1Char('\n'));
}

}  // namespace spall
