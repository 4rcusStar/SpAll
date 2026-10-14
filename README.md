# SpAll

A lightweight **native desktop GLSL editor with in-process SPIR-V compilation and
disassembly**.

> Write GLSL → compile to SPIR-V with one key → immediately see the disassembly
> (SPIR-V Assembly) and the reflection data (descriptors / push constants / IO), with
> compiler errors mapped back to source line and column.

It is not a full IDE and not a renderer: **there is no graphics preview and no Vulkan
driver involvement**. The point is to make the edit → compile → inspect loop fast and
legible.

glslang, SPIRV-Tools and SPIRV-Reflect are all **linked statically into the process**.
No external executable is launched at runtime.

---

## 1. Features

| Area | Capability |
|---|---|
| Editor | `QPlainTextEdit` with a line-number gutter, current-line highlight, bracket matching, auto-indent, tabs or spaces, `Ctrl+/` comment toggle, find and replace, unsaved-changes marker |
| File browsing | A **full folder tree** (`QFileSystemModel`) in the left dock: switchable root folder, filter by name; double-click or Enter opens any supported shader (`.vert/.frag/.comp/.geom/.tesc/.tese/.rgen/...`), and unsupported files explain why they cannot be opened. A shortcut list of the bundled `examples/` sits below it. Up to 10 recent files |
| Syntax highlighting | Comments (including multi-line block comments), preprocessor directives, string and number literals, types, qualifiers, control flow, builtin functions, builtin variables, function definition names, plus **full-line and squiggly diagnostic underlines** |
| Completion | Pops up automatically after 2 characters; `Ctrl+Space` triggers it manually; candidates merge *identifiers in the current document / local function signatures / the builtin function table / types and keywords*; `↑`/`↓`/`PgUp`/`PgDn` move the highlight; **`Tab` or `Enter` accepts the highlighted candidate** (not the first one); a mouse click inserts the row that was clicked; `Esc` closes it; after `(` or `,` the status bar shows the parameter signature |
| Syntax hints | `#version` placement, bracket matching, suspected missing semicolons (conservative, reported as errors), common mistakes such as `vec4f`/`float4`/`mat3x3`/`sampler2d`/`texture2D(`/`gl_FragColor`, a missing `main()`, and a vertex shader that never writes `gl_Position` |
| Compilation | In-process glslang; stage / `#version` / Vulkan version / automatic location and binding mapping / resource limits / optional recursive `#include` expansion with line-number remapping. Runs on a **background thread**, one job at a time, and a newer request invalidates the older result |
| Disassembly | `spvBinaryToText`; indent, friendly names, comments, no header, byte offsets, nested indentation and block reordering are all selectable. Shows word count, instruction count, id bound and SPIR-V version. Double-clicking an `OpBranch %label` jumps to its `OpLabel`. Modules over 2 MB load in chunks so the UI stays responsive |
| Export | **Save the `.spv` binary** (`Ctrl+Shift+E`, or the *Save .spv* button in the assembly panel) as a module you can hand straight to `spirv-val`, `spirv-dis` or a driver; export `.spvasm` text (`Ctrl+E`); copy the whole assembly to the clipboard (`Ctrl+Shift+C`) |
| Validation | `spvValidateWithOptions`, target environment selectable as Vulkan 1.0–1.3 or Universal 1.0/1.3/1.6 |
| Optimization | `spvOptimizerRun` with *None / Performance (-O) / Size (-Os)*. Optimization runs **before** disassembly, and both the pre- and post-optimization word counts are shown |
| Reflection | Two-phase SPIRV-Reflect enumeration: descriptor bindings (with UBO/SSBO member expansion showing offset, size, padded size and type), push constants, stage inputs and outputs (location, component, format, builtin), specialization constants and execution info. *Copy as Text* exports an aligned table; *Generate C++ Struct* emits a std140 struct with `static_assert`s |
| Stats / Log | Top 20 instructions plus the number of distinct opcodes (counted through `spvBinaryParse`), glslang and optimizer logs, and a hex dump of the optimized module |
| Problems list | Severity / category / location / message, double-click or Enter to jump, `F8` and `Shift+F8` for next and previous |
| Persistence | Window geometry, dock layout, theme, font, compile defaults, recent files, browser root folder and UI language are all stored via `QSettings` |

### Language

The interface ships in **English (the source language) and Simplified Chinese**, switchable at runtime from **View → Language**, the *Interface* page of the settings dialog, or the `--lang=<code>` command-line option. No restart is required: the whole window retranslates in place, including every open editor and panel.

```powershell
.\SpAll.exe --lang=zh_CN     # start in Chinese for this run only
.\SpAll.exe --lang=en        # force English
```

`--lang` overrides the persisted setting without changing it, which is convenient for screenshots and review. The choice made through the menu or the settings dialog *is* persisted.

Adding another language takes three steps: add one row to `Translator::available()`, run the `SpAll-lupdate` build target to extract the strings, and translate the generated `.ts`. The build compiles it to `.qm` with `lrelease` and copies it into `translations/` next to the executable.

The catalogue round-trip has two helper scripts, because the `.ts` format is XML and hand-editing ~520 entries is error-prone:

```powershell
python tools/ts_extract.py translations/spall_zh_CN.ts build/worklist.txt   # numbered list
#   ... translate, producing a JSON map ...
python tools/ts_apply.py   translations/spall_zh_CN.ts translations/zh_CN.json
```

`ts_apply.py` matches on *context + source text*, so a translation survives re-running `lupdate` (which renumbers and reorders entries).

---

## 2. Repository layout

```
SpAll/
  CMakeLists.txt
  cmake/
    Dependencies.cmake        # dependency fetching (FetchContent or a local checkout)
    generate_moc.cmake        # build-time Qt moc generation (replaces AUTOMOC, see §7)
    deploy_qt.cmake           # post-build Qt runtime copy (replaces windeployqt, see §7)
  core/                       # pure C++ + QtCore, no Widgets; builds and tests standalone
    ShaderStage.*             # stage enum <-> glslang EShLanguage / file suffixes
    CompileOptions.h          # aggregate of compilation settings
    Diagnostic.h/.cpp
    CompileResult.h
    ReflectData.h/.cpp        # reflection data + "copy as text" rendering
    IncludePreprocessor.*     # recursive #include expansion with line mapping
    GlslCompiler.*            # glslang wrapper, diagnostic parsing, line remapping
    SpirvDisassembler.*       # disassembly / validation / optimization / stats (RAII over the C API)
    SpirvReflection.*         # SPIRV-Reflect wrapper
  ui/                         # Qt Widgets
    MainWindow.*              # toolbar, docks, background compile orchestration, static hints
    EditorTab.*               # one tab per file (one stage) plus the line-number gutter
    GlslHighlighter.*         # syntax highlighting (light and dark themes)
    GlslCompleter.*           # completion model and parameter hints
    GlslLanguageData.*        # keyword / builtin function / builtin variable tables
    Translator.*              # UI language catalogues and runtime switching
    StartupTrace.*            # opt-in startup milestone log (see §7)
    SettingsDialog.*          # settings UI and QSettings persistence
    panels/AsmPanel.*         # disassembly, highlighting, label jumps, chunked loading
    panels/ReflectPanel.*     # reflection tree, copy as text, C++ struct generation
    panels/DiagnosticsPanel.* # problems list
    panels/OutputPanel.*      # stats / log / binary
  app/main.cpp                # entry point (glslang lifetime, --selftest)
  translations/
    spall_zh_CN.ts            # Chinese catalogue (compiled to .qm at build time)
    zh_CN.json                # the same translations as a reviewable JSON map
  tests/
    smoke_main.cpp            # GUI-less smoke test, SpAll_smoke (acceptance)
    hint_check_main.cpp       # missing-semicolon heuristic regression test
    contrast_check_main.cpp   # diagnostic tint contrast test
    completion_check_main.cpp # completion unit test (model, prefix, keys, caret positions)
    completion_gui_check_main.cpp # completion through EditorTab, incl. popup geometry
    file_browser_check_main.cpp   # file browser test
    spv_save_check_main.cpp   # .spv saving test
    reflect_dump_main.cpp     # debugging helper (not built by default)
    cross_validate_main.cpp   # spirv-dis cross-validation helper (not built by default)
  tools/
    bootstrap_qt.ps1          # unattended Qt 6 installation
    fetch_wheels.py           # offline wheelhouse builder
    install_qt_singleprocess.py # single-process Qt installer
    fetch_deps.ps1            # fetch and pin the four dependency checkouts
    build.ps1                 # one-shot configure + build
    ts_extract.py             # dump a .ts catalogue as a numbered work list
    ts_apply.py               # write translations from JSON back into a .ts
  examples/                   # basic.vert / basic.frag / ubo.frag / comp.comp
  third_party/                # dependency sources (offline mode)
  .qt/                        # local Qt installation (created by bootstrap_qt.ps1)
  build/                      # build tree (generated)
```

---

## 3. Dependencies

| Dependency | Version (pinned tag) | Purpose |
|---|---|---|
| Qt 6 | ≥ 6.5; tested with 6.9.3 `msvc2022_64` | Widgets / Gui / Core / Concurrent |
| [SPIRV-Headers](https://github.com/KhronosGroup/SPIRV-Headers) | `vulkan-sdk-1.4.304.0` | SPIR-V headers |
| [SPIRV-Tools](https://github.com/KhronosGroup/SPIRV-Tools) | `vulkan-sdk-1.4.304.0` | Disassembly / validation / optimization |
| [glslang](https://github.com/KhronosGroup/glslang) | `vulkan-sdk-1.4.304.0` (== glslang 15.1.0) | GLSL front end |
| [SPIRV-Reflect](https://github.com/KhronosGroup/SPIRV-Reflect) | `vulkan-sdk-1.4.304.0` | Reflection |

> **All four dependencies use the same Vulkan SDK tag.** The originally specified
> combination of glslang 15.3.0 with SPIRV-Tools 1.4.304.0 **does not compile**:
> glslang 15.3.0's `SPIRV/SpvTools.cpp` calls
> `spvValidatorOptionsSetAllowOffsetTextureOperand()`, which does not exist in
> 1.4.304.0's `libspirv.h`. Pinning all four to one SDK tag gives the exact combination
> Khronos releases and tests together. Link order remains a hard requirement:
> **SPIRV-Headers → SPIRV-Tools → glslang**.

Toolchain: CMake ≥ 3.22 and a C++20 compiler (MSVC 2022, GCC or Clang). On Windows,
VS 2022 is recommended since it bundles CMake and Ninja.

---

## 4. Building

### 4.1 Preparing Qt (if it is not installed yet)

```powershell
pwsh -File tools/bootstrap_qt.ps1
```

This installs Qt 6.9.3 into `<project>/.qt/6.9.3/msvc2022_64`. It does three unusual
things, each explained in the script's own comments:

* `pip` on the target machine cannot reach any package index (the resolver hangs
  indefinitely), so `tools/fetch_wheels.py` (standard library `urllib` only) first
  downloads aqtinstall and its dependencies into a local wheelhouse, which is then
  installed with `pip install --no-index --find-links`;
* aqtinstall's `install-qt` spawns a `multiprocessing` pool, and creating the named
  pipes that requires is refused in restricted environments, so
  `tools/install_qt_singleprocess.py` drives aqt's download and unpack APIs in a single
  process instead;
* it uses the Tsinghua mirror by default (its `Updates.xml` is byte-for-byte identical to
  the official one; the measured sha256 matches upstream exactly).

On Linux, just install the distribution packages, e.g.
`sudo apt install qt6-base-dev cmake g++`.

### 4.2 Preparing the third-party sources (optional, for offline builds)

```powershell
pwsh -File tools/fetch_deps.ps1
```

This shallow-clones the four dependencies at their pinned tags into `third_party/`.
**CMake does not touch the network when local copies exist**; otherwise it falls back to
fetching them with `FetchContent`.

### 4.3 Building

```powershell
# Windows (locates the CMake and vcvars64 bundled with Visual Studio)
pwsh -File tools/build.ps1                      # Debug
pwsh -File tools/build.ps1 -Config Release      # Release

# Manual / Linux
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_PREFIX_PATH=/path/to/Qt/6.9.3/gcc_64
cmake --build build -j
```

Artifacts land in `build/bin/<Config>/`, separated per configuration so that Debug and
Release binaries of the same name never overwrite each other:

| File | Description |
|---|---|
| `SpAll` / `SpAll.exe` | the GUI application |
| `SpAll_smoke` / `SpAll_smoke.exe` | the GUI-less smoke test |

The build copies `examples/` next to the executable, so running straight from the build
tree still shows the example list on the left.

Common options:

| Option | Default | Description |
|---|---|---|
| `SPALL_BUILD_UI` | ON | Turn off to build only the core library and the smoke test (no Qt Widgets needed) |
| `SPALL_BUILD_TESTS` | ON | Build `SpAll_smoke` |
| `SPALL_DEPLOY_QT` | ON | Copy the Qt runtime next to the executable after building (**on by default, so the binary is double-clickable**) |
| `SPALL_DEPS_FETCH_TAGS` | ON | Turn off to fail instead of downloading when a dependency is missing |
| `GLSL_TOOLS_DEPS_DIR` | `<src>/third_party` | Root of the local dependency checkouts |
| `SPALL_BUILD_REFLECT_DUMP` | OFF | Build the reflection debugging helper |
| `SPALL_BUILD_CROSS_VALIDATE` | OFF | Build the `spirv-dis` cross-validation helper |
| `SPALL_BUILD_HINT_CHECK` | OFF | Build the missing-semicolon heuristic regression test |
| `SPALL_BUILD_CONTRAST_CHECK` | OFF | Build the diagnostic tint contrast test |
| `SPALL_BUILD_COMPLETION_CHECK` | OFF | Build the completion tests (unit and GUI) |
| `SPALL_BUILD_SPV_SAVE_CHECK` | OFF | Build the `.spv` saving test |
| `SPALL_BUILD_FILE_BROWSER_CHECK` | OFF | Build the file browser test |

### 4.4 Running

The build copies the Qt runtime next to the executable (see `SPALL_DEPLOY_QT`), so
**`SpAll.exe` can simply be double-clicked** with no `PATH` configuration:

```
build/bin/Release/
  SpAll.exe                 <- double-click this
  Qt6Core.dll  Qt6Gui.dll  Qt6Widgets.dll  Qt6Concurrent.dll
  platforms/qwindows.dll    <- required; without it Qt exits with "no Qt platform plugin"
  styles/...  imageformats/...
  translations/
    spall_zh_CN.qm          <- Chinese UI catalogue
  examples/                 <- backs the file list on the left
```

Debug builds copy the debug variants with a `d` suffix (`Qt6Cored.dll`,
`platforms/qwindowsd.dll`, …). The two configurations are fully independent.

To go back to managing `PATH` by hand:

```powershell
cmake -S . -B build -DSPALL_DEPLOY_QT=OFF
```

You can then open `build/SpAll.sln` in Visual Studio and press F5 (the debugger `PATH`
is already configured), or add `<project>\.qt\6.9.3\msvc2022_64\bin` to `PATH`
yourself.

---

## 5. Keyboard shortcuts

| Shortcut | Action |
|---|---|
| `F7` / `Ctrl+B` | Compile the current tab |
| `Ctrl+Shift+B` | Compile every tab |
| `Ctrl+Space` | Trigger completion (filtered by the prefix if there is one, otherwise list everything) |
| `Tab` / `Enter` | Accept the highlighted candidate while the popup is open |
| `Esc` | Close the completion popup |
| `Ctrl+F` | Find in the editor or the assembly panel |
| `Ctrl+S` / `Ctrl+Shift+S` | Save / Save As |
| `F8` / `Shift+F8` | Next / previous problem |
| `Ctrl+1..4` | Switch the right-hand tab (assembly / reflection / stats·log) |
| `Ctrl+E` | Export the assembly as `.spvasm` |
| `Ctrl+Shift+E` | **Save the SPIR-V binary as `.spv`** |
| `Ctrl+Shift+C` | Copy the whole assembly to the clipboard |
| `Ctrl+/` | Toggle line comments |
| `Ctrl+O` / `Ctrl+N` | Open / New |

---

## 6. Acceptance tests

### 6.1 Smoke test (no GUI)

```powershell
$env:Path = "<project>\.qt\6.9.3\msvc2022_64\bin;$env:Path"
.\build\bin\Debug\SpAll_smoke.exe
```

It runs the whole pipeline over `examples/` and covers:

1. all four examples compile, word count > 5, magic == `0x07230203`, id bound > 0;
2. `spvValidate` returns `SPV_SUCCESS`;
3. disassembly produces text containing `OpEntryPoint` and `Vertex`;
4. reflection reports the expected descriptor and push-constant counts, UBO
   `set=0 binding=0` with member `offset`s matching the source `layout`, compute
   `local_size == 8x8`, and `basic.vert` output containing the builtin `Position`;
5. **negative cases**: a deliberately broken shader reports errors with line numbers in
   the expected range; empty source does not crash; an error inside an `#include` is
   **remapped to `common.glsl:2`**; disabling automatic mapping without writing
   `layout(binding=)` makes validation fail; `-O` lowers the word count and the
   optimized module still validates; the *None* level leaves the module untouched.

Any failure exits non-zero and prints the reason. On success it prints
`61 / 61 checks passed`.

### 6.2 Cross-validation against `spirv-dis`

Requires a local Vulkan SDK:

```powershell
cmake -S . -B build -DSPALL_BUILD_CROSS_VALIDATE=ON
cmake --build build --config Release
$env:Path = "<project>\.qt\6.9.3\msvc2022_64\bin;$env:Path"
.\build\bin\Release\SpAll_cross_validate.exe examples\basic.vert vert out\vert
& "$env:VULKAN_SDK\Bin\spirv-dis.exe" out\vert.spv -o out\vert.sdk.spvasm
# out\vert.cli.spvasm matches this tool's default disassembly options line for line
& "$env:VULKAN_SDK\Bin\spirv-val.exe" --target-env vulkan1.3 out\vert.spv
```

The helper writes three files: `.spv` (the module itself), `.cli.spvasm` (equivalent to
`spirv-dis` defaults: indented with friendly names) and `.plain.spvasm` (indentation
only, numeric ids preserved).

**Measured result (see §8):** every example validates with `SPV_SUCCESS`, and this
tool's disassembly is **identical line for line** to the SDK's `spirv-dis`
(67 / 79 / 71 lines).

### 6.3 Manual verification checklist

| Step | Expected |
|---|---|
| Compile `examples/basic.vert` | The assembly panel shows `OpEntryPoint Vertex` and `OpStore`; reflection includes the builtin `Position` (as a member of the `gl_PerVertex` block) |
| Open `examples/ubo.frag` | Reflection shows `set=0 binding=0 uniform buffer Material`, `binding=1 combined image sampler albedo` and a push-constant block of size 16; UBO member `tint` has offset 0 |
| Delete a semicolon / use an undeclared variable | The problems list shows an error, the matching line is tinted and underlined, and double-clicking jumps to the right line and column |
| An error inside an `#include` | The error is remapped to the real line number of the included file |
| Disable automatic mapping and omit `layout(binding=)` | Compilation or validation reports an error, proving the validation path is live |
| Switch optimization to `-O` | The status bar shows both word counts and the assembly changes |
| Edit the source | The assembly panel immediately reports that it is stale and prompts for F7, instead of passing off the old module |

---

## 7. Known limitations and implementation notes

* **Build-time moc**: CMake's own AUTOMOC spawns moc through libuv
  (`cmake -E cmake_autogen`), which fails in restricted environments with
  `libuv process spawn failed: operation not permitted`. `cmake/generate_moc.cmake`
  therefore invokes `moc.exe` directly at configure and build time, regenerating
  incrementally from header timestamps. This is equivalent and portable.
* **Generator**: the build uses the `Visual Studio 17 2022` generator. Ninja hangs at
  zero CPU on every build step (including CMake's `try_compile`) in restricted
  environments, because the named pipes it needs to talk to its children are refused.
  `tools/build.ps1` therefore uses the VS generator.
* **Qt runtime deployment**: Qt is installed as shared libraries, so the executable
  needs Qt's DLLs. `cmake/deploy_qt.cmake` copies the required DLLs and plugins next to
  the executable after each build (on by default), which is what makes double-clicking
  work. **`windeployqt` is deliberately not used**: it spawns helper processes such as
  `qtpaths` through pipes (`Unable to query qtpaths: Error running binary qtpaths:
  pipe:`), which fails in restricted environments; copying a fixed manifest is more
  reliable.
* **Not a single-file executable**: after the DLLs are copied, the result is still a
  directory. True single-file distribution would require a static Qt build.
* **`#include` line remapping**: the mapping table is line-accurate; an error inside an
  included file points at that file's real path and line number, while the UI stays on
  the tab that pulled it in (the included file is not opened automatically).
* **Specialization constant defaults**: SPIRV-Reflect does not expose the evaluated
  result of constant instructions, so the specialization-constant column shows
  `(not provided)` rather than guessing a default.
* **Syntax hints are heuristics**: there is no full semantic analysis, and they can be
  switched off in the settings dialog. The missing-semicolon check in particular is
  **conservative**; its rules are below, and a false positive can simply be ignored
  because the compiler's verdict is authoritative.
* **Diagnostic tinting and underlines**: both are drawn by
  `QPlainTextEdit::setExtraSelections`, **not** by the syntax highlighter. The reason is
  that `QSyntaxHighlighter::setFormat` replaces the formats of a range wholesale, so
  using it to tint would also wipe the syntax foreground colours and produce white text
  on a pale tint. The tint uses `FullWidthSelection` to span the row and specifies its
  foreground colour explicitly. Measured contrast: light theme 8.3:1 (error) and 8.6:1
  (warning); dark theme 9.7:1 and 9.4:1 — all above the 4.5:1 WCAG AA threshold. Covered
  by `tests/SpAll_contrast_check`.
* **Missing-semicolon rules** (`hints::findMissingSemicolons` in `core/Diagnostic.cpp`):
  `//` and multi-line `/* */` comments are stripped first, and a line is only flagged
  when it is inside a `{}` block, has balanced brackets, does not end in
  `; { } , : \ ?` or an operator, does not start with `if/for/while/switch/else/do`, and
  the previous line does not end in `( [ , \ ?` or an operator (that is, it is not a
  continuation). It is then reported at **error** severity.
  `tests/SpAll_hint_check` pins 11 counter-examples (including an `if (...)` with a
  trailing comment and a `mat3(` spanning lines) and 5 positive cases.
* **Three things completion gets right on purpose** (all in `ui/GlslCompleter.cpp`,
  covered by `tests/SpAll_completion_check`):
  1. a custom model must implement `Qt::EditRole`, because that is the role `QCompleter`
     matches the prefix against; with only `DisplayRole` the match set is always empty
     and the popup never appears;
  2. while the editor holds focus, `QCompleter` does not forward `↑`/`↓` to the popup
     (it only does that for the `QLineEdit`/`QComboBox` it manages), so **navigation
     keys only** must be forwarded explicitly; `setTabChangesFocus(false)` is needed as
     well, otherwise `Tab` is treated as focus navigation and never reaches the key
     handler;
  3. `Tab`, `Enter` and mouse clicks all insert by **name** (`Qt::EditRole`); a model row
     must never be used to index the candidate array, because the popup uses
     `QCompleter`'s proxy model whose row order differs from the source list. That was
     the cause of "clicking always inserts the same word".
* **Popup placement and size**: `QCompleter::complete(rect)` derives both from that one
  rect — `QCompleterPrivate::showPopup()` takes `w = rect.width()` and
  `rh = rect.height()`, puts the popup at the screen coordinates of `rect.bottomLeft()`,
  then calls `popup->setGeometry(pos.x(), pos.y(), w, h)`. The rect must therefore
  satisfy **both** of these:
  1. **size taken from the popup's own `sizeHint()`**. Passing the caret rect alone
     makes the popup as wide as a caret (about 1px): it reports itself visible while
     being too narrow to read or click. An empty rect anchors the wrong corner and gets
     stretched across the whole widget, and padding the height by row count is wrong too
     because the height takes part in the flip-above decision.
  2. **position aligned by bottom-left**. `QRect::bottomLeft()` is
     `(x, y + height - 1)`, so the rect has to be positioned with
     `moveBottomLeft(caretRect.bottomLeft())`. Writing `QRect(x, y + caretHeight, w, h)`
     puts the bottom-left a whole popup-height below the caret, which pushes the popup
     far down the screen — that was the cause of "the popup is not under the cursor".
  `tests/SpAll_completion_gui_check` pins this with four geometry assertions (width,
  height, and position relative to the caret), and `SpAll.exe --selftest` prints the
  popup's offset from the caret's bottom-left.
* **`Tab` accepts the highlighted row**: the accept key is intercepted in **two** places,
  and both are required.
  1. `GlslCompleter::eventFilter` (installed on the editor) — the path taken while the
     editor holds focus;
  2. `ShaderEdit::keyPressEvent` — a visible popup grabs the keyboard and re-delivers the
     key to the editor through `QWidget::event()`, and **that path does not run event
     filters**.
  Implementing only the first one misses `Tab` in real use, so it falls through to
  indentation — which is exactly what "Tab still indents" looked like. `--selftest`
  presses `Tab` once through each route and asserts the outcome.
* **Accepting a candidate must never silently do nothing**: `acceptCurrentCompletion()`
  resolves the name with three fallbacks — the view's highlighted row, then
  `QCompleter`'s current index, then row 0 of the filtered model — and finally the first
  candidate in the list. `nameFromIndex()` also strips the `(signature)` from a display
  string when the model implements only `DisplayRole`. This matters because the caller
  may have **already hidden the popup** by the time the key is handled; if resolution
  then failed and returned early, the user would see the list vanish and the document
  stay unchanged. `tests/SpAll_completion_check` pins this in four assertions, including
  "acceptance still inserts the first candidate when nothing is highlighted".
* **How the completion prefix is read**: the identifier under the caret is scanned
  directly out of the current line's text rather than via
  `QTextCursor::WordUnderCursor`. The latter depends on Qt's word-boundary heuristic
  agreeing with the caret position, and when the two disagree the prefix comes back
  empty, so the "prefix shorter than 2 characters" branch silently hides the popup —
  which also looks like "the completion popup does not appear".
  `tests/SpAll_completion_check` pins this with 10 caret-position cases.
* **`--selftest`**: `SpAll.exe --selftest` opens the real window, creates a scratch tab
  and types a prefix through real key events, then checks whether the popup appeared and
  offered candidates, printing the outcome and returning it as the exit code (also
  written to `spall-selftest-report.txt` next to the executable). It covers wiring that
  a unit test cannot reproduce: the popup is opened from `textChanged` on a window that
  is **really on screen**. The scratch tab is discarded afterwards, and the popup is
  closed — otherwise `app.quit()` closes the window, `closeEvent()` raises the modal
  "unsaved changes" prompt for a modified tab, and with nobody to answer it the process
  would sit there forever instead of returning its exit code. This self-test needs a real
  display platform; do not run it under `QT_QPA_PLATFORM=offscreen`.
* **Completion troubleshooting switch**: with the environment variable
  `SPALL_COMPLETION_TRACE=1` set, the completion path appends every decision to
  `spall-completion-trace.log` next to the executable: the prefix read in
  `onTextChanged`, the popup's candidate count and final geometry, every key the
  `eventFilter` sees together with its origin (editor or popup) and `popupVisible`, and
  the name resolved when a candidate is accepted. SpAll is a GUI-subsystem binary whose
  stderr a shell frequently cannot read, so a file is the only reliable way to observe
  it. Nothing is written unless the variable is set.

  To capture it (PowerShell):

  ```powershell
  cd <directory containing the exe>
  $env:SPALL_COMPLETION_TRACE = "1"
  .\SpAll.exe
  # reproduce the problem, close the application, then read .\spall-completion-trace.log
  ```
* **Startup tracing**: a GUI-subsystem binary has no console, so a problem *before* the
  first window exists leaves nothing to inspect. With `SPALL_STARTUP_TRACE=1` set,
  `main()` and the `MainWindow` constructor append milestones to
  `spall-startup-trace.log` next to the executable, which shows exactly how far startup
  got before stalling. Two real bugs were found this way:

  1. `Translator::install()` broadcasts `QEvent::LanguageChange` to every widget in
     `QApplication::allWidgets()`. Calling it part-way through the `MainWindow`
     constructor therefore delivered the event to a half-built window, and
     `retranslate()` dereferenced members that did not exist yet — an immediate hang with
     no window ever appearing. The catalogue is now installed at the *end* of the
     constructor, and `retranslate()` also returns early if its members are not there.
  2. A GUI-subsystem binary's `stderr` is not reliably readable from a shell even when
     redirected, which is why the `--selftest` report is written to a file.
* **Language plumbing**: `lupdate` extracts strings statically, and it does **not** see
  through a helper that forwards its argument to `QCoreApplication::translate()`. The
  `core/` library has no `QObject` (so no `tr()`), and its ~330 strings silently failed
  to appear in the catalogue until each call site was changed to pass the text through
  `QT_TRANSLATE_NOOP` explicitly:

  ```cpp
  QString trs(const char* text) { return QCoreApplication::translate("Compiler", text); }
  // at every call site:
  trs(QT_TRANSLATE_NOOP("Compiler", "The source is empty, so there is nothing to compile."))
  ```

  The same applies to the GLSL description table, whose entries reach the translator
  through a function parameter, so all 162 of them are wrapped in `QT_TRANSLATE_NOOP`
  inside the table. `tools/ts_extract.py` reports the message count, which is how the
  omission was caught: the catalogue had 257 entries when it should have had 522.
* **Not implemented**: graphics preview, a Vulkan/OpenGL runtime, GPU execution
  verification, HLSL/Slang/WGSL front ends, SPIR-V cross-compilation, LSP, remote or
  multi-user operation, and a plugin system.

---

## 8. Measured results (on the development machine)

Environment: Windows 11 (10.0.22631), Visual Studio 2022 Community 17.14 (MSVC 19.44),
CMake 3.31.6, Qt 6.9.3 `msvc2022_64`, Vulkan SDK 1.4.341.1.

| Item | Result |
|---|---|
| Debug build | ✅ passes |
| Release build | ✅ passes |
| `SpAll_smoke` (Debug) | ✅ **61 / 61 checks passed**, exit code 0 |
| `SpAll_smoke` (Release) | ✅ **61 / 61 checks passed**, exit code 0 |
| GUI startup | ✅ main window created (title `Untitled — SpAll`) |
| Double-click launch (Qt not on `PATH`) | ✅ Debug and Release both start standalone |
| Diagnostic tint contrast | ✅ 8.3–9.7:1, above the 4.5:1 WCAG AA threshold |
| Missing-semicolon heuristic | ✅ 16 / 16 cases (11 counter-examples + 5 positive) |
| Completion | ✅ 52 / 52 checks (prefix filtering, `↑`/`↓` highlight movement, `Tab`/`Enter` accepting the **highlighted** row, click inserting the clicked row, prefix detection at 10 caret positions, popup width/height/position relative to the caret, acceptance still inserting the first candidate when nothing is highlighted) |
| `.spv` saving | ✅ 15 / 15 checks; SDK `spirv-val` returns `SPV_SUCCESS` for all 3 examples |
| File browser | ✅ 16 / 16 checks (extension set agrees with the stage mapping) |
| `SpAll.exe --selftest` | ✅ exit code 0 in Debug and Release; popup 256×24, offset from the caret's bottom-left dx=−21 dy=−1 (the horizontal offset is Qt pulling the popup back inside the screen edge) |
| UI language switching | ✅ `--lang=en` → `Untitled — SpAll`; `--lang=zh_CN` → `未命名 — SpAll`; `--selftest` reports `catalogue=active` in Chinese and `n/a (source language)` in English |
| Translation catalogue | ✅ 522 / 522 strings finished, 0 unfinished (`lrelease`), 55 KB `spall_zh_CN.qm` |
| All tests combined | ✅ **164 / 164 checks** (once in Debug and once in Release) |
| `spirv-val` (SDK) | ✅ 3 / 3 modules `SPV_SUCCESS` |
| `spirv-dis` (SDK) line-by-line comparison | ✅ 3 / 3 modules identical: 67 / 79 / 71 lines |
| Optimization `-O` | ✅ 167 → 88 words, and the optimized module still validates |
| `#include` line remapping | ✅ error located at `common.glsl:2` |
| `-Os` level | ✅ runs successfully |
| Optimization level *None* | ✅ leaves the module untouched |

### Source size

| Directory | Files | Lines |
|---|---|---|
| `core/` | 17 | 2760 |
| `ui/` | 26 | 5808 |
| `app/` | 1 | 50 |
| `tests/` | 9 | 1757 |
| **Own C++ total** | **53** | **≈ 10375** |

> The original brief suggested staying under roughly 6000 lines. The result is about
> 10375. The excess comes mainly from the complete GLSL type / qualifier / builtin
> function tables the brief requires (`GlslLanguageData.cpp` alone is ~560 lines), the
> basic syntax hints and completion it also requires, and the reflection panel's member
> expansion, std140 struct generation and text export. Every feature is genuinely
> implemented rather than stubbed; if size matters, the data tables could move to an
> external resource file.

---

## 9. License

The third-party dependencies keep their own original licenses (all Khronos
Apache-2.0 / BSD-style). The source in this repository is intended for learning and
local tooling use.
