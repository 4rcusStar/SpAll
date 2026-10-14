// IncludePreprocessor.h — recursive #include expansion with line remapping.
//
// glslang has no notion of files: it compiles a single source string. To make
// #include work (and to still report errors against the user's real file and
// line), the source is expanded here first, and every emitted line records where
// it came from. After compilation the mapping is used to rewrite glslang's line
// numbers back to the original file.
#pragma once

#include <QHash>
#include <QString>
#include <QStringList>

#include "Diagnostic.h"

namespace spall {

/// Where one line of the expanded source came from.
struct SourceLine {
    QString file;  ///< absolute path of the originating file
    int line = 0;  ///< 1-based line within that file
};

/// Records the origin of each physical line of the expanded source.
class LineMap {
public:
    /// Append the origin of one emitted line.
    void append(const QString& file, int line);
    /// Number of mapped lines.
    int size() const { return m_lines.size(); }

    /// Resolve an expanded 1-based line to its original file and line.
    /// Returns false when the line is out of range.
    bool resolve(int expandedLine, QString* file, int* line) const;

    /// True when every line still maps to `mainFile` at the same number
    /// (i.e. no #include was expanded and no remapping is needed).
    bool isIdentity(const QString& mainFile) const;

private:
    QVector<SourceLine> m_lines;
};

/// Result of expanding a source file.
struct PreprocessResult {
    bool ok = false;
    QString source;              ///< expanded source handed to glslang
    LineMap map;                 ///< expanded line -> original file/line
    QVector<Diagnostic> diagnostics;
    QStringList includedFiles;   ///< every file that was pulled in
};

/// Expands #include directives recursively.
class IncludePreprocessor {
public:
    /// Depth limit guarding against pathological include chains.
    static constexpr int kMaxDepth = 32;

    /// Expand `source` taken from `mainFile`.
    ///
    /// Search order for "..." and <...> includes: the including file's directory
    /// first, then `includePaths` in order.
    ///
    /// When `enabled` is false the source is passed through unchanged and the
    /// line map is filled with an identity mapping, so callers can always use
    /// the map.
    PreprocessResult process(const QString& source,
                             const QString& mainFile,
                             const QStringList& includePaths,
                             bool enabled) const;

    /// Rewrite a diagnostic's line from expanded coordinates to original ones.
    static void remap(Diagnostic* diagnostic, const LineMap& map, const QString& mainFile);

private:
    struct Impl;
    void expand(const QString& file,
                const QString& source,
                const QStringList& includePaths,
                QString* out,
                LineMap* map,
                PreprocessResult* result,
                QStringList* stack) const;
};

}  // namespace spall
