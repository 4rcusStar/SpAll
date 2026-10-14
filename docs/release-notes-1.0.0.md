# Release notes — SpAll 1.0.0

First public release.

## What it is

A lightweight desktop **GLSL editor that compiles to SPIR-V in-process** and shows the
result immediately: disassembly, reflection data and compiler diagnostics, without
leaving the window. There is deliberately no graphics preview and no Vulkan runtime —
the tool is scoped to one tight loop: **edit → compile → inspect**.

glslang, SPIRV-Tools and SPIRV-Reflect are linked statically into the process, so no
external executable is ever launched.

## Highlights

- **Editing** — line-number gutter, current-line highlight, bracket matching,
  auto-indent, comment toggling, find and replace, unsaved-change tracking.
- **Completion and hints** — candidates merged from the document, local function
  signatures, a builtin function table and the GLSL keyword set; a conservative
  heuristic pass flags `#version` placement, bracket mismatches, likely missing
  semicolons and common porting mistakes before the compiler runs.
- **Compilation** — stage, `#version`, Vulkan target, resource limits and optional
  recursive `#include` expansion with line-number remapping, executed on a worker
  thread so the UI stays responsive.
- **Disassembly and validation** — selectable SPIRV-Tools output options, label jumps,
  chunked loading for large modules, validation against any Vulkan or Universal target
  environment, and `-O` / `-Os` optimization with before/after word counts.
- **Reflection** — descriptor bindings with UBO/SSBO member expansion, push constants,
  stage inputs and outputs, specialization constants and execution info, exportable as
  aligned text or a std140 C++ struct.
- **Export** — the compiled `.spv` binary, the `.spvasm` text, or the assembly to the
  clipboard.
- **Interface** — English and Simplified Chinese, switchable at runtime with no restart
  (`--lang=zh_CN` also forces it for a single run).

## Requirements

CMake ≥ 3.22, a C++20 compiler, and Qt 6 (≥ 6.5). On Windows, Visual Studio 2022 is
recommended as it bundles CMake.

Dependencies are pinned to a single Vulkan SDK tag, `vulkan-sdk-1.4.304.0` — the
combination Khronos actually releases and tests together. Static link order is a hard
requirement: **SPIRV-Headers → SPIRV-Tools → glslang**.

## Building from source

```powershell
pwsh -File tools/bootstrap_qt.ps1     # Qt, if not already installed (into .qt/)
pwsh -File tools/fetch_deps.ps1       # dependency sources (optional; enables offline builds)
pwsh -File tools/build.ps1 -Config Release
```

See the README for the full option list and the Linux equivalent.

## Verification

- **164 checks** across seven test programs, passing in both Debug and Release.
- The smoke test runs the full pipeline over `examples/` and validates every module
  with `spirv-val` (`SPV_SUCCESS`).
- Disassembly is byte-identical to the Vulkan SDK's `spirv-dis` for the bundled
  examples.
- `SpAll --selftest` drives the real window through genuine key events and fails with a
  non-zero exit code if the completion popup misbehaves.

## Notes

- **Source-only release.** No prebuilt binaries: Qt and the four Khronos dependencies
  are not redistributed here.
- Released under the **MIT** license; the third-party dependencies retain their original
  Apache-2.0 / BSD-style licenses.
- Windows-centric: the build tooling ships as PowerShell scripts, though the CMake
  project itself is portable and builds on Linux with distribution Qt packages.
