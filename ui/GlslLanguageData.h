// GlslLanguageData.h — keyword / builtin-function / builtin-variable tables.
//
// Single source of truth shared by the syntax highlighter and the completer, so
// a keyword can never be highlighted but not completed (or vice versa).
#pragma once

#include <QString>
#include <QStringList>
#include <QVector>

namespace spall {

/// One completion candidate.
struct CompletionItem {
    QString name;
    QString returnType;   ///< empty for non-functions
    QString signature;    ///< full parameter list, e.g. "(sampler2D, vec2)"
    QString description;  ///< one-line explanation shown as a tooltip

    /// "name(signature)" for functions, plain "name" otherwise.
    QString displaySignature() const;
};

/// One `layout(...)` qualifier name.
struct LayoutQualifier {
    const char* name;
    const char* description;
};

/// Static GLSL 4.50 language data.
class GlslLanguageData {
public:
    // --- keyword groups (used for highlighting) -----------------------------

    /// void bool int uint float double
    static const QStringList& scalarTypes();
    /// vec2..vec4, ivec*, uvec*, bvec*, dvec*
    static const QStringList& vectorTypes();
    /// mat2..mat4, mat2x3..mat4x3, dmat*
    static const QStringList& matrixTypes();
    /// sampler*, isampler*, usampler*, image*, subpassInput*, texture*, atomic_uint
    static const QStringList& opaqueTypes();
    /// const in out inout uniform buffer shared ...
    static const QStringList& qualifiers();
    /// if else for while do switch case default break continue return discard struct true false precision
    static const QStringList& controlFlow();
    /// All type keywords combined (for the highlighter).
    static QStringList allTypes();

    /// Names valid inside layout(...).
    static const QVector<LayoutQualifier>& layoutQualifiers();

    /// gl_* builtin variables.
    static const QStringList& builtinVariables();

    /// Preprocessor directive names (without the leading '#').
    static const QStringList& preprocessorDirectives();

    // --- completion ---------------------------------------------------------

    /// All builtin functions with signature and description.
    static const QVector<CompletionItem>& builtinFunctions();

    /// Types as completion candidates.
    static QVector<CompletionItem> typeCompletions();

    /// Keywords as completion candidates.
    static QVector<CompletionItem> keywordCompletions();

    /// Look up a builtin function by exact name; nullptr when unknown.
    static const CompletionItem* findBuiltinFunction(const QString& name);
};

}  // namespace spall
