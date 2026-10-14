// CompileOptions.cpp — option labels, defaults and result bookkeeping.
#include "CompileOptions.h"

#include "CompileResult.h"

#include <QCoreApplication>

#include <algorithm>

namespace spall {

namespace {

// Localised string for this translation context.
//
// A macro rather than a function on purpose: lupdate parses sources statically
// and cannot see through a call that passes the text on to
// QCoreApplication::translate(). Spelling the lookup out here means every trs(QT_TRANSLATE_NOOP("CompileOptions", ...))
// call site is extracted automatically, while the runtime behaviour is unchanged.
/// Localised string for this translation context.
///
/// The text is passed through QT_TRANSLATE_NOOP at every call site, which is what
/// lets lupdate extract it statically; this function then performs the runtime
/// lookup. core/ has no QObject, so tr() is not available there.
QString trs(const char* text) {
    return QCoreApplication::translate("CompileOptions", text);
}

}  // namespace

QString targetEnvName(TargetEnv env) {
    switch (env) {
        case TargetEnv::Vulkan10:    return QStringLiteral("Vulkan 1.0");
        case TargetEnv::Vulkan11:    return QStringLiteral("Vulkan 1.1");
        case TargetEnv::Vulkan12:    return QStringLiteral("Vulkan 1.2");
        case TargetEnv::Vulkan13:    return QStringLiteral("Vulkan 1.3");
        case TargetEnv::Universal10: return QStringLiteral("Universal 1.0");
        case TargetEnv::Universal13: return QStringLiteral("Universal 1.3");
        case TargetEnv::Universal16: return QStringLiteral("Universal 1.6");
    }
    return QStringLiteral("Vulkan 1.3");
}

QString optimizationLevelName(OptimizationLevel level) {
    switch (level) {
        case OptimizationLevel::None:        return trs(QT_TRANSLATE_NOOP("CompileOptions", "None"));
        case OptimizationLevel::Performance: return trs(QT_TRANSLATE_NOOP("CompileOptions", "Performance -O"));
        case OptimizationLevel::Size:        return trs(QT_TRANSLATE_NOOP("CompileOptions", "Size -Os"));
    }
    return trs(QT_TRANSLATE_NOOP("CompileOptions", "None"));
}

QString vulkanVersionName(VulkanVersion version) {
    switch (version) {
        case VulkanVersion::Vulkan10: return QStringLiteral("1.0");
        case VulkanVersion::Vulkan11: return QStringLiteral("1.1");
        case VulkanVersion::Vulkan12: return QStringLiteral("1.2");
        case VulkanVersion::Vulkan13: return QStringLiteral("1.3");
    }
    return QStringLiteral("1.3");
}

QString spirvVersionName(SpirvVersion version) {
    const int raw = static_cast<int>(version);
    return QStringLiteral("%1.%2").arg(raw / 10000).arg((raw % 10000) / 100);
}

SpirvVersion defaultSpirvVersionFor(VulkanVersion vulkan) {
    // SPIR-V 1.6 requires Vulkan 1.3; older targets stick to 1.3 for the widest
    // tool and driver compatibility.
    return vulkan == VulkanVersion::Vulkan13 ? SpirvVersion::Spv16 : SpirvVersion::Spv13;
}

TargetEnv defaultTargetEnvFor(VulkanVersion vulkan) {
    switch (vulkan) {
        case VulkanVersion::Vulkan10: return TargetEnv::Vulkan10;
        case VulkanVersion::Vulkan11: return TargetEnv::Vulkan11;
        case VulkanVersion::Vulkan12: return TargetEnv::Vulkan12;
        case VulkanVersion::Vulkan13: return TargetEnv::Vulkan13;
    }
    return TargetEnv::Vulkan13;
}

// --- CompileResult -----------------------------------------------------------

QByteArray CompileResult::optimizedBytes() const {
    const QVector<quint32>& words = spirvOptimized.isEmpty() ? spirv : spirvOptimized;
    QByteArray bytes;
    bytes.resize(static_cast<int>(words.size() * sizeof(quint32)));
    if (!words.isEmpty()) {
        memcpy(bytes.data(), words.constData(), static_cast<size_t>(bytes.size()));
    }
    return bytes;
}

void CompileResult::recountDiagnostics() {
    errorCount = 0;
    warningCount = 0;
    hintCount = 0;
    for (const Diagnostic& diag : diagnostics) {
        switch (diag.severity) {
            case Severity::Error:   ++errorCount; break;
            case Severity::Warning: ++warningCount; break;
            case Severity::Info:
            case Severity::Hint:    ++hintCount; break;
        }
    }
}

QVector<Diagnostic> CompileResult::sortedDiagnostics() const {
    QVector<Diagnostic> sorted = diagnostics;
    std::stable_sort(sorted.begin(), sorted.end(), [](const Diagnostic& a, const Diagnostic& b) {
        if (a.severity != b.severity) {
            return static_cast<int>(a.severity) < static_cast<int>(b.severity);
        }
        if (a.line != b.line) {
            return a.line < b.line;
        }
        return a.column < b.column;
    });
    return sorted;
}

}  // namespace spall
