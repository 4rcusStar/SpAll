# SpAll build helper
#
# CMake is not on PATH on this machine, so this script locates the copy bundled
# with Visual Studio, enters the MSVC environment via vcvars64, and then
# configures and/or builds SpAll.
#
# Generator: "Visual Studio 17 2022". Ninja is deliberately NOT used: in this
# environment Ninja hangs at zero CPU during any build step, including CMake's
# own try_compile, because the sandbox denies the named pipe Ninja uses to talk
# to its child processes. MSBuild has no such dependency.
#
# Qt is expected in <project>/.qt/<version>/msvc2022_64 (see tools/bootstrap_qt.ps1)
# or anywhere reachable through -QtPrefix.
#
# Usage:
#   pwsh -File tools/build.ps1                       # configure + build Debug
#   pwsh -File tools/build.ps1 -Config Release
#   pwsh -File tools/build.ps1 -Target SpAll_smoke
#   pwsh -File tools/build.ps1 -ConfigureOnly
#   pwsh -File tools/build.ps1 -Clean

[CmdletBinding()]
param(
    [ValidateSet("Debug", "Release", "RelWithDebInfo")]
    [string]$Config = "Debug",
    [string]$Target = "",
    [string]$BuildDir = "",
    [string]$QtPrefix = "",
    [string]$DepsDir = "",
    [switch]$ConfigureOnly,
    [switch]$Clean
)

$ErrorActionPreference = "Stop"

$ProjectRoot = Split-Path -Parent $PSScriptRoot
$CacheRoot   = Join-Path (Split-Path -Parent $ProjectRoot) ".cache"

if (-not $BuildDir) { $BuildDir = Join-Path $ProjectRoot "build" }

# --- locate the VS-bundled toolchain -----------------------------------------
$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
if (-not (Test-Path $vswhere)) { throw "vswhere.exe not found; Visual Studio 2022 is required." }

$vsPath = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (-not $vsPath) { throw "No Visual Studio installation with the C++ toolset was found." }
$vsPath = $vsPath.Trim()

$cmake = Join-Path $vsPath "Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
$vcvars = Join-Path $vsPath "VC\Auxiliary\Build\vcvars64.bat"

foreach ($tool in @($cmake, $vcvars)) {
    if (-not (Test-Path $tool)) { throw "Required tool missing: $tool" }
}

# --- locate Qt ---------------------------------------------------------------
if (-not $QtPrefix) {
    $candidates = @()
    $qtRoot = Join-Path $ProjectRoot ".qt"
    if (Test-Path $qtRoot) {
        $candidates += (Get-ChildItem $qtRoot -Directory | Sort-Object Name -Descending |
                        ForEach-Object { Join-Path $_.FullName "msvc2022_64" })
    }
    $candidates += @("E:\Qt\6.9.3\msvc2022_64", "C:\Qt\6.9.3\msvc2022_64")
    foreach ($candidate in $candidates) {
        if (Test-Path (Join-Path $candidate "lib\cmake\Qt6\Qt6Config.cmake")) {
            $QtPrefix = $candidate
            break
        }
    }
}
if (-not $QtPrefix) {
    throw @"
Qt 6 not found.
What to do: run tools/bootstrap_qt.ps1 to install it into <project>/.qt, or pass
-QtPrefix <path to .../msvc2022_64>.
"@
}

Write-Host "== SpAll build ==" -ForegroundColor Cyan
Write-Host "   project  : $ProjectRoot"
Write-Host "   build dir: $BuildDir"
Write-Host "   config   : $Config"
Write-Host "   Qt       : $QtPrefix"
Write-Host "   cmake    : $cmake"
Write-Host "   vcvars   : $vcvars"

if ($Clean -and (Test-Path $BuildDir)) {
    Write-Host "   removing $BuildDir" -ForegroundColor Yellow
    Remove-Item -Recurse -Force $BuildDir
}

$depsArg = @()
if ($DepsDir) { $depsArg = @("-DGLSL_TOOLS_DEPS_DIR=$DepsDir") }

$configureArgs = @(
    "-S", $ProjectRoot,
    "-B", $BuildDir,
    "-G", "`"Visual Studio 17 2022`"",
    "-A", "x64",
    "-DCMAKE_PREFIX_PATH=$QtPrefix"
) + $depsArg

# Run CMake inside the MSVC environment.
$cmd = "call `"$vcvars`" >nul 2>&1 && `"$cmake`" " + ($configureArgs -join " ")
Write-Host "`n-- configure --" -ForegroundColor Yellow
cmd /c $cmd
if ($LASTEXITCODE -ne 0) { throw "CMake configure failed with exit code $LASTEXITCODE" }

if ($ConfigureOnly) {
    Write-Host "`nConfigure only; done." -ForegroundColor Green
    exit 0
}

# NOTE: neither a trailing "--" nor "--parallel" is used here.
#   * A trailing "--" makes CMake forward the rest of the arguments to MSBuild in
#     a form it rejects; the build then fails with a bare exit code 1.
#   * "--parallel" makes CMake drive MSBuild in parallel mode, which uses an
#     inter-process channel that is refused in restricted environments; MSBuild
#     then exits immediately with code 1 and prints nothing.
# MSBuild still builds independent projects concurrently on its own.
$buildArgs = @("--build", $BuildDir, "--config", $Config)
if ($Target) {
    # cmake --build --target accepts a single target name.
    $buildArgs = @("--build", $BuildDir, "--config", $Config, "--target", $Target)
}

# Native commands write warnings to stderr; Windows PowerShell 5.1 escalates that
# to a terminating error while the preference is "Stop".
$previousPreference = $ErrorActionPreference
$ErrorActionPreference = "Continue"
try {
    $configureCmd = "call `"$vcvars`" >nul 2>&1 && `"$cmake`" " + ($configureArgs -join " ")
    Write-Host "`n-- configure --" -ForegroundColor Yellow
    cmd /c $configureCmd
    $configureExit = $LASTEXITCODE

    if ($configureExit -eq 0 -and -not $ConfigureOnly) {
        $buildCmd = "call `"$vcvars`" >nul 2>&1 && `"$cmake`" " + ($buildArgs -join " ")
        Write-Host "`n-- build --" -ForegroundColor Yellow
        cmd /c $buildCmd
        $buildExit = $LASTEXITCODE
    }
} finally {
    $ErrorActionPreference = $previousPreference
}

if ($configureExit -ne 0) { throw "CMake configure failed with exit code $configureExit" }

if ($ConfigureOnly) {
    Write-Host "`nConfigure only; done." -ForegroundColor Green
    exit 0
}

if ($buildExit -ne 0) { throw "Build failed with exit code $buildExit" }

Write-Host "`nBuild finished. Binaries in $BuildDir\bin\$Config" -ForegroundColor Green
foreach ($dir in @((Join-Path $BuildDir "bin\$Config"), (Join-Path $BuildDir "bin"))) {
    if (Test-Path $dir) {
        Get-ChildItem $dir -File -Include *.exe, *.dll -ErrorAction SilentlyContinue |
            Select-Object Name, Length | Format-Table -AutoSize | Out-String -Width 120 | Write-Host
    }
}
