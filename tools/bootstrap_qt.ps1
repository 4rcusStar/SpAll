# SpAll Qt 6 bootstrap
#
# Installs Qt 6 (MSVC 2022 64-bit desktop, which includes Core/Gui/Widgets) into
# -QtRoot without an interactive Qt account. Uses aqtinstall driven through a
# local, offline wheelhouse.
#
# Why a wheelhouse: on this host `pip install` cannot reach any package index
# (its resolver stalls indefinitely), while plain HTTP downloads work. So we
# download every wheel with tools/fetch_wheels.py (stdlib urllib only) and then
# install with `pip --no-index --find-links`.
#
# Usage:
#   pwsh -File tools/bootstrap_qt.ps1
#   pwsh -File tools/bootstrap_qt.ps1 -QtVersion 6.9.3 -QtRoot E:\Qt
#
# Notes:
#   * aqtinstall always touches $env:APPDATA\aqt for its cache and temp dirs, and
#     that location may be unwritable. We redirect APPDATA/TMP/TEMP into the
#     workspace so nothing outside it is touched.
#   * The Qt mirror defaults to the Tsinghua mirror, which serves byte-identical
#     repository metadata to download.qt.io and is dramatically faster here.

[CmdletBinding()]
param(
    [string]$QtVersion  = "6.9.3",
    [string]$QtRoot     = "",
    [string]$Arch       = "win64_msvc2022_64",
    [string]$Mirror     = "https://mirrors.tuna.tsinghua.edu.cn/qt",
    [string]$CacheRoot  = "",
    [switch]$ForceReinstall
)

$ErrorActionPreference = "Stop"

$ProjectRoot = Split-Path -Parent $PSScriptRoot
if (-not $CacheRoot) { $CacheRoot = Join-Path (Split-Path -Parent $ProjectRoot) ".cache" }
# Default to a project-local prefix: it keeps the checkout self-contained and
# stays inside the writable workspace (E:\Qt is outside it).
if (-not $QtRoot)    { $QtRoot = Join-Path $ProjectRoot ".qt" }

$wheelhouse = Join-Path $CacheRoot "wheelhouse"
$pylibs     = Join-Path $CacheRoot "pylibs"
$appdata    = Join-Path $CacheRoot "appdata"
$tempdir    = Join-Path $CacheRoot "tmp"
$aqtarch    = Join-Path $CacheRoot "aqtarch"

foreach ($d in @($wheelhouse, $pylibs, $appdata, $tempdir, $aqtarch)) {
    New-Item -ItemType Directory -Force -Path $d | Out-Null
}

Write-Host "== SpAll Qt bootstrap ==" -ForegroundColor Cyan
Write-Host "   Qt version : $QtVersion ($Arch)"
Write-Host "   Qt root    : $QtRoot"
Write-Host "   cache root : $CacheRoot"

# --- 1. Is Qt already there? -------------------------------------------------
$qtPrefix = Join-Path $QtRoot "$QtVersion\msvc2022_64"
$qtConfig = Join-Path $qtPrefix "lib\cmake\Qt6\Qt6Config.cmake"
if ((Test-Path $qtConfig) -and -not $ForceReinstall) {
    Write-Host "`nQt already installed: $qtConfig" -ForegroundColor Green
    Write-Host "Use -ForceReinstall to reinstall."
    exit 0
}

# --- 2. Build the offline wheelhouse ----------------------------------------
$aqtPresent = $false
if (Test-Path (Join-Path $pylibs "aqt")) { $aqtPresent = $true }

if (-not $aqtPresent) {
    Write-Host "`n== Building offline wheelhouse (pip cannot reach an index on this host) ==" -ForegroundColor Yellow
    & python (Join-Path $PSScriptRoot "fetch_wheels.py") --wheelhouse $wheelhouse aqtinstall
    if ($LASTEXITCODE -ne 0) { throw "wheelhouse download failed" }

    Write-Host "`n== Installing aqtinstall offline ==" -ForegroundColor Yellow
    & python -m pip install --no-index --no-warn-script-location `
        --find-links $wheelhouse --target $pylibs aqtinstall --progress-bar off
    if ($LASTEXITCODE -ne 0) { throw "offline aqtinstall install failed" }
} else {
    Write-Host "`n== aqtinstall already present in $pylibs ==" -ForegroundColor Green
}

# --- 3. Point aqt at a mirror and keep all state inside the cache root -------
$env:PYTHONPATH = $pylibs
$env:APPDATA    = $appdata
$env:TMP        = $tempdir
$env:TEMP       = $tempdir

$aqtIni = Join-Path $pylibs "aqt\settings.ini"
if (Test-Path $aqtIni) {
    $ini = Get-Content $aqtIni -Raw
    $ini = $ini -replace '(?m)^baseurl\s*:.*$',              "baseurl : $Mirror"
    $ini = $ini -replace '(?m)^archive_download_location\s*:.*$', "archive_download_location : $aqtarch"
    Set-Content -Path $aqtIni -Value $ini -NoNewline
}

Write-Host "`n== aqtinstall ==" -ForegroundColor Yellow
& python -m aqt version

# --- 4. Install Qt ----------------------------------------------------------
Write-Host "`n== Installing Qt $QtVersion ($Arch) into $QtRoot ==" -ForegroundColor Yellow
Write-Host "   (large download; archives are cached in $aqtarch)"

# aqt's own `install-qt` command is not used here: it builds a
# multiprocessing.Manager() queue, which needs named pipes that this sandbox
# denies. install_qt_singleprocess.py drives aqt's download/extract helpers in
# one process instead. See that file's docstring.
#
# No --modules flag on purpose: Qt Core/Gui/Widgets all live in the base
# "qtbase" package, so there is no separate 'qtwidgets' module to request.
$qtArgs = @(
    (Join-Path $PSScriptRoot "install_qt_singleprocess.py"),
    "--pythonpath", $pylibs,
    "--version",    $QtVersion,
    "--arch",       $Arch,
    "--outputdir",  $QtRoot,
    "--archivedir", $aqtarch,
    "--base",       $Mirror
)
& python @qtArgs
if ($LASTEXITCODE -ne 0) { throw "Qt installation failed with exit code $LASTEXITCODE" }

if (Test-Path $qtConfig) {
    Write-Host "`nQt installed successfully." -ForegroundColor Green
    Write-Host "Configure SpAll with:"
    Write-Host "  -DCMAKE_PREFIX_PATH=$qtPrefix"
} else {
    throw "Qt install finished but $qtConfig is missing"
}
