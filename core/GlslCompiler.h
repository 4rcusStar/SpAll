// GlslCompiler.h — in-process GLSL -> SPIR-V compilation via glslang.
//
// glslang is a process-global singleton: InitializeProcess() must be called once
// before any compile and FinalizeProcess() once at shutdown. `initialize()` /
// `finalize()` below wrap that, and `isInitialized()` lets callers report a
// friendly error instead of crashing.
#pragma once

#include <QString>

#include "CompileOptions.h"
#include "CompileResult.h"

namespace spall {

class GlslCompiler {
public:
    GlslCompiler();
    ~GlslCompiler();

    GlslCompiler(const GlslCompiler&) = delete;
    GlslCompiler& operator=(const GlslCompiler&) = delete;

    /// Call once at application start. Returns false if glslang failed to
    /// initialize (the process is then unusable for compilation).
    static bool initialize();
    /// Call once at application exit. Safe to call without a matching initialize.
    static void finalize();
    /// True between a successful initialize() and finalize().
    static bool isInitialized();

    /// Compile `source`, which came from `fileName`.
    ///
    /// Handles #include expansion, glslang parsing/linking, SPIR-V generation and
    /// diagnostic parsing. Never throws; failures come back as diagnostics plus a
    /// two-part (what happened + what to do) failure message.
    ///
    /// `sourceAlreadyPreprocessed` skips the #include pass, for callers that
    /// pre-expanded the source themselves.
    CompileResult compile(const QString& source,
                          const QString& fileName,
                          const CompileOptions& options) const;

private:
    /// Parse glslang's info log into diagnostics, remapping #include line numbers.
    void parseInfoLog(const QString& log,
                      DiagSource source,
                      const class LineMap& map,
                      const QString& mainFile,
                      CompileResult* result) const;
};

}  // namespace spall
