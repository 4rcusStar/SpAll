# SpAll

A lightweight desktop GLSL editor that compiles to SPIR-V in-process and shows you the
result immediately.

Write GLSL, press <kbd>F7</kbd>, and read the disassembly, the reflection data and any
compiler diagnostics without leaving the window. There is no graphics preview and no
Vulkan runtime: the tool is deliberately scoped to one tight loop — **edit → compile →
inspect**.

glslang, SPIRV-Tools and SPIRV-Reflect are linked statically into the process, so no
external executable is ever launched.

---

## Features

**Editing.** `QPlainTextEdit` with a line-number gutter, current-line highlight, bracket
matching, auto-indent, comment toggling, find and replace, and unsaved-change tracking.

**Completion and hints.** Candidates are merged from the current document, local function
signatures, a builtin function table and the GLSL keyword set. A conservative heuristic
pass flags `#version` placement, bracket mismatches, likely missing semicolons and a
handful of common porting mistakes before the compiler even runs.

**Compilation.** Stage, `#version`, Vulkan target, resource limits and optional recursive
`#include` expansion with line-number remapping. Jobs run on a worker thread, one at a
time, and a newer request invalidates the older result.

**Disassembly and validation.** Selectable SPIRV-Tools output options, label jumps, and
chunked loading for large modules. SPIR-V validation is available against any Vulkan or
Universal target environment, and the optimizer supports both `-O` and `-Os` with
before/after word counts.

**Reflection.** Descriptor bindings with UBO/SSBO member expansion, push constants,
stage inputs and outputs, specialization constants and execution info — exportable as
aligned text or as a std140 C++ struct.

**Export.** The compiled `.spv` binary, the `.spvasm` text, or the assembly to the
clipboard.

**Interface.** English and Simplified Chinese, switchable at runtime with no restart.

---

## Building

Requirements: CMake ≥ 3.22, a C++20 compiler, and Qt 6 (≥ 6.5). On Windows, Visual
Studio 2022 is recommended since it bundles CMake.

```powershell
# Qt, if it is not installed yet (installs into .qt/ inside the project)
pwsh -File tools/bootstrap_qt.ps1

# Dependency sources are fetched automatically, or pin them locally for offline builds
pwsh -File tools/fetch_deps.ps1

pwsh -File tools/build.ps1 -Config Release
```

On Linux the equivalent is:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=/path/to/Qt/6.9.3/gcc_64
cmake --build build -j
```

Artifacts are written to `build/bin/<Config>/`. Qt is deployed next to the executable by
default, so the binary can be launched directly — the bundled `examples/` are copied
alongside it as well.

| Option | Default | Effect |
|---|---|---|
| `SPALL_BUILD_UI` | `ON` | Build the Qt front-end; turn off for the core library and smoke test only |
| `SPALL_DEPLOY_QT` | `ON` | Copy the Qt runtime next to the executable |
| `SPALL_BUILD_TESTS` | `ON` | Build the smoke test |
| `SPALL_DEPS_FETCH_TAGS` | `ON` | Turn off to fail rather than download a missing dependency |
| `SPALL_BUILD_*_CHECK` | `OFF` | Build the individual regression test programs |

---

## Dependencies

All four are pinned to a single Vulkan SDK tag, `vulkan-sdk-1.4.304.0` — the combination
Khronos actually releases and tests together:

| Dependency | Purpose |
|---|---|
| [glslang](https://github.com/KhronosGroup/glslang) | GLSL front end |
| [SPIRV-Tools](https://github.com/KhronosGroup/SPIRV-Tools) | Disassembly, validation, optimization |
| [SPIRV-Headers](https://github.com/KhronosGroup/SPIRV-Headers) | SPIR-V headers |
| [SPIRV-Reflect](https://github.com/KhronosGroup/SPIRV-Reflect) | Reflection |

Static link order is a hard requirement: **SPIRV-Headers → SPIRV-Tools → glslang**.

---

## Language

The interface ships in English — the source language — and Simplified Chinese, switchable
from **View → Language**, the settings dialog, or the command line:

```sh
SpAll --lang=zh_CN     # start in Chinese for this run only
```

Adding a language means one row in `Translator::available()`, a run of the
`SpAll-lupdate` target to extract the strings, and a translated `.ts`; the build compiles
it to `.qm` and deploys it.

---

## Testing

Seven test programs cover the compiler pipeline, diagnostics, completion, the file
browser and `.spv` export — **164 checks**, passing in both Debug and Release. The smoke
test runs the whole pipeline over `examples/` and validates the modules against
`spirv-val`.

```sh
cmake -S . -B build -DSPALL_BUILD_COMPLETION_CHECK=ON -DSPALL_BUILD_HINT_CHECK=ON
cmake --build build --config Release
./build/bin/Release/SpAll_smoke
```

`SpAll --selftest` additionally drives the real window — typing through genuine key
events to confirm the completion popup opens and accepts correctly — and returns a
non-zero exit code if any expectation fails.

---

## Design notes

A few constraints shaped the implementation and are worth knowing before touching the
relevant code:

* **`QCompleter` is easy to misuse.** It matches on `EditRole`, sizes its popup from the
  same rect it uses for placement, and does not forward navigation keys for a
  `QPlainTextEdit`. Each mistake yields a plausible-looking but broken popup.
* **A visible popup owns the keyboard.** Accepting a candidate must be handled both in
  the completer's event filter and in the editor's `keyPressEvent`, because the popup
  re-delivers keys along a path that never runs event filters.
* **`core/` has no `QObject`,** so its messages call `QCoreApplication::translate` and
  carry `QT_TRANSLATE_NOOP` at the call site for the extractor to see them.
* **Translation is installed last** in the window constructor, since the broadcast it
  performs reaches objects that are still being constructed.
* **Build tooling avoids spawning helper processes** — moc, Qt deployment and
  translation compilation are all driven directly, which keeps the build working in
  restricted environments.

---

## License

Third-party dependencies retain their original licenses (Apache-2.0 / BSD-style, all
from Khronos). No license has been chosen for this project's own source yet.
