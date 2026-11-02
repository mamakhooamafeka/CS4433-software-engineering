# Makes the Windows release package: a zip a school technician can unzip and run,
# with no compiler or vcpkg needed on the school computer (docs/INSTALL.md).
# Run it from the project folder after a Release build:
#   powershell -ExecutionPolicy Bypass -File tools\package-windows.ps1
# CI runs it on every push (ci.yml) and on every release tag (release.yml).
param(
    [string]$Version = "",
    [string]$BuildDir = "build\Release",
    [string]$VcpkgRoot = "",
    [string]$OutDir = "dist"
)
$ErrorActionPreference = "Stop"
Set-Location (Split-Path -Parent $PSScriptRoot)

if ($Version -eq "") {
    # Read the version from CMakeLists.txt: project(ModeratedLMS VERSION 1.0.0 ...)
    $m = Select-String -Path CMakeLists.txt -Pattern 'project\(\S+ VERSION (\d+\.\d+\.\d+)'
    $Version = $m.Matches[0].Groups[1].Value
}
if ($VcpkgRoot -eq "") {
    if ($env:VCPKG_INSTALLATION_ROOT) { $VcpkgRoot = $env:VCPKG_INSTALLATION_ROOT } else { $VcpkgRoot = "C:\vcpkg" }
}

$exe = Join-Path $BuildDir "lms.exe"
if (-not (Test-Path $exe)) { throw "$exe not found. Build first: cmake --build build --config Release" }

$name = "ModeratedLMS-$Version-windows-x64"
$stage = Join-Path $OutDir $name
if (Test-Path $stage) { Remove-Item $stage -Recurse -Force }
New-Item -ItemType Directory -Force -Path $stage, "$stage\docs", "$stage\licences" | Out-Null

# 1. The program and the library DLLs that vcpkg copied next to it.
Copy-Item $exe $stage
$dlls = Get-ChildItem (Join-Path $BuildDir "*.dll")
foreach ($need in "sqlite3", "sodium") {
    if (-not ($dlls | Where-Object { $_.Name -like "*$need*" })) { throw "No $need DLL in $BuildDir" }
}
$dlls | Copy-Item -Destination $stage

# 2. The Visual C++ runtime DLLs, copied next to lms.exe (Microsoft's "app-local"
#    deployment), so the school computer does not need the redistributable installer.
$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
$crt = $null
if (Test-Path $vswhere) {
    $vs = & $vswhere -latest -products * -property installationPath
    if ($vs) {
        $crt = Get-ChildItem "$vs\VC\Redist\MSVC\*\x64\Microsoft.VC14*.CRT" -Directory -ErrorAction SilentlyContinue |
               Sort-Object FullName -Descending | Select-Object -First 1
    }
}
if ($crt) {
    Copy-Item "$($crt.FullName)\*.dll" $stage
} elseif ($env:CI) {
    throw "Visual C++ runtime DLLs not found on the build machine; a release package must include them."
} else {
    Write-Warning "Visual C++ runtime DLLs not found; the school computer will need vc_redist.x64.exe (INSTALL.md section 12)."
}

# 3. Start-up and backup scripts, documentation and third-party licences.
Copy-Item tools\start-lms.bat, tools\copy-backups.bat $stage
Copy-Item docs\INSTALL.md, docs\RELEASE_NOTES.md "$stage\docs"
Copy-Item docs\manuals\*.md "$stage\docs"
foreach ($port in "sqlite3", "libsodium") {
    $c = Join-Path $VcpkgRoot "installed\x64-windows\share\$port\copyright"
    if (Test-Path $c) { Copy-Item $c "$stage\licences\$port.txt" }
    else { Write-Warning "Licence file for $port not found at $c" }
}

$zip = Join-Path $OutDir "$name.zip"
if (Test-Path $zip) { Remove-Item $zip -Force }
Compress-Archive -Path $stage -DestinationPath $zip
Write-Host "Package ready: $zip"
Get-ChildItem $stage -Recurse | Select-Object -ExpandProperty FullName
