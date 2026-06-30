param([switch]$Clean)

$ErrorActionPreference = "Stop"
$root     = Split-Path $PSScriptRoot -Parent
$toolDir  = Join-Path $root "tools\slr-convert"
$buildDir = Join-Path $root "build\slr-convert"

if ($Clean -and (Test-Path $buildDir)) {
    Remove-Item -Recurse -Force $buildDir
    Write-Host "Cleaned build directory." -ForegroundColor Yellow
}

New-Item -ItemType Directory -Force -Path $buildDir | Out-Null

Write-Host "Configuring slr-convert..." -ForegroundColor Cyan
cmake -S $toolDir -B $buildDir -G "Visual Studio 17 2022" -A x64
if ($LASTEXITCODE -ne 0) { Write-Error "CMake configure failed"; exit 1 }

Write-Host "Building slr-convert..." -ForegroundColor Cyan
cmake --build $buildDir --config Release
if ($LASTEXITCODE -ne 0) { Write-Error "Build failed"; exit 1 }

$exe = Join-Path $buildDir "Release\slr-convert.exe"
if (Test-Path $exe) {
    Write-Host ""
    Write-Host "Build successful!" -ForegroundColor Green
    Write-Host "Executable: $exe" -ForegroundColor White
    Write-Host ""
    Write-Host "Usage:" -ForegroundColor Yellow
    Write-Host "  $exe piano.sf2"
    Write-Host "  $exe strings.sfz strings.slr --verbose"
    Write-Host "  $exe drums.sf2 drums.slr --gain -3"
    Write-Host ""
    Write-Host "Place .slr output in: Documents\Solara\Presets\"
} else {
    Write-Error "Executable not found after build"
}
