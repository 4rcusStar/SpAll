# SpAll dependency vendoring
#
# Clones the four pinned third-party sources into <project>/third_party so the
# build runs fully offline. cmake/Dependencies.cmake prefers these local copies
# and only falls back to FetchContent when they are absent.
#
# Pinned revisions (must stay in sync with cmake/Dependencies.cmake):
#   SPIRV-Headers   vulkan-sdk-1.4.304.0
#   SPIRV-Tools     vulkan-sdk-1.4.304.0
#   glslang         vulkan-sdk-1.4.304.0   (== glslang 15.1.0)
#   SPIRV-Reflect   vulkan-sdk-1.4.304.0
#
# One SDK tag for all four: glslang 15.3.0 does not compile against
# SPIRV-Tools 1.4.304.0 (it calls a validator option added later).
#
# Usage:
#   pwsh -File tools/fetch_deps.ps1
#   pwsh -File tools/fetch_deps.ps1 -Force     # wipe and re-clone

[CmdletBinding()]
param(
    [string]$ThirdPartyDir = "",
    [string]$SpirvHeadersTag = "vulkan-sdk-1.4.304.0",
    [string]$SpirvToolsTag   = "vulkan-sdk-1.4.304.0",
    [string]$GlslangTag      = "vulkan-sdk-1.4.304.0",
    [string]$SpirvReflectTag = "vulkan-sdk-1.4.304.0",
    [switch]$Force
)

$ErrorActionPreference = "Stop"

# Windows PowerShell 5.1 turns any stderr text from a native command into a
# terminating NativeCommandError when the preference is "Stop". git writes its
# progress to stderr, so temporarily relax the preference around git calls.
function Invoke-Git {
    param([Parameter(ValueFromRemainingArguments = $true)][string[]]$GitArgs)
    $previous = $ErrorActionPreference
    $ErrorActionPreference = "Continue"
    try {
        & git @GitArgs 2>&1 | ForEach-Object { Write-Host "   $_" }
        return $LASTEXITCODE
    } finally {
        $ErrorActionPreference = $previous
    }
}

$ProjectRoot = Split-Path -Parent $PSScriptRoot
if (-not $ThirdPartyDir) { $ThirdPartyDir = Join-Path $ProjectRoot "third_party" }
New-Item -ItemType Directory -Force -Path $ThirdPartyDir | Out-Null

$deps = @(
    @{ Name = "SPIRV-Headers"; Dir = "spirv_headers"; Tag = $SpirvHeadersTag },
    @{ Name = "SPIRV-Tools";   Dir = "spirv_tools";   Tag = $SpirvToolsTag },
    @{ Name = "glslang";       Dir = "glslang";       Tag = $GlslangTag },
    @{ Name = "SPIRV-Reflect"; Dir = "spirv_reflect"; Tag = $SpirvReflectTag }
)

Write-Host "== SpAll dependency vendoring ==" -ForegroundColor Cyan
Write-Host "   target: $ThirdPartyDir"

foreach ($dep in $deps) {
    $dest = Join-Path $ThirdPartyDir $dep.Dir
    $url = "https://github.com/KhronosGroup/$($dep.Name).git"

    if ($Force -and (Test-Path $dest)) {
        Write-Host "`n-- removing $($dep.Dir)" -ForegroundColor Yellow
        Remove-Item -Recurse -Force $dest
    }

    if (Test-Path (Join-Path $dest ".git")) {
        Write-Host "`n-- $($dep.Name): already present, verifying tag $($dep.Tag)" -ForegroundColor Green
        $current = (& git -C $dest describe --tags --exact-match 2>$null)
        if ($current -eq $dep.Tag) {
            Write-Host "   at $($dep.Tag) - skipping"
            continue
        }
        Write-Host "   on '$current'; fetching and checking out $($dep.Tag)"
        $null = Invoke-Git -C $dest fetch --depth 1 origin "refs/tags/$($dep.Tag):refs/tags/$($dep.Tag)"
        $null = Invoke-Git -C $dest checkout -q "tags/$($dep.Tag)"
        continue
    }

    Write-Host "`n-- $($dep.Name) @ $($dep.Tag)" -ForegroundColor Yellow
    # A shallow clone of a tag keeps this fast; these trees are large.
    $exit = Invoke-Git clone --depth 1 --branch $dep.Tag $url $dest
    if ($exit -ne 0) { throw "clone of $($dep.Name) failed with exit code $exit" }
}

Write-Host "`n== result ==" -ForegroundColor Cyan
foreach ($dep in $deps) {
    $dest = Join-Path $ThirdPartyDir $dep.Dir
    $tag = if (Test-Path $dest) { (& git -C $dest describe --tags --exact-match 2>$null) } else { "<missing>" }
    $ok = if ($tag -eq $dep.Tag) { "OK " } else { "!! " }
    Write-Host ("  {0}{1,-16} {2}" -f $ok, $dep.Dir, $tag)
}
