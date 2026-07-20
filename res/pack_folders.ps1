param(
    [string]$ResDir = $PSScriptRoot
)

Set-Location -LiteralPath $ResDir
$ErrorActionPreference = "Stop"

$tools = @(
    @{ Folder = "CPUZ";              Exe = "cpuz_x64.exe" },
    @{ Folder = "BatteryInfoView";   Exe = "BatteryInfoView.exe" },
    @{ Folder = "CoreTemp64";        Exe = "Core Temp.exe" },
    @{ Folder = "hwinfo";            Exe = "HWiNFO64.exe" },
    @{ Folder = "CrystalDiskMark";   Exe = "DiskMark64.exe" }
)

Add-Type -AssemblyName System.Drawing

foreach ($tool in $tools) {
    $folder = $tool.Folder
    $exeName = $tool.Exe
    $packFile = "$folder.pack"

    Write-Host "Packing $folder -> $packFile ..."

    $allFiles = Get-ChildItem -LiteralPath $folder -Recurse -File
    if ($allFiles.Count -eq 0) {
        Write-Warning "  No files found in $folder, skipping."
        continue
    }

    $stream = [System.IO.File]::Create($packFile)
    $writer = [System.IO.BinaryWriter]::new($stream)

    $writer.Write([uint32]$allFiles.Count)

    foreach ($file in $allFiles) {
        $relativePath = $file.FullName.Substring($ResDir.Length + 1)
        $innerPath = $relativePath.Substring($folder.Length + 1)
        $pathBytes = [System.Text.Encoding]::Unicode.GetBytes($innerPath)
        $dataBytes = [System.IO.File]::ReadAllBytes($file.FullName)

        $writer.Write([uint16]($innerPath.Length))
        $writer.Write($pathBytes)
        $writer.Write([uint32]$dataBytes.Length)
        $writer.Write($dataBytes)

        Write-Host "  + $innerPath ($($dataBytes.Length) bytes)"
    }

    $writer.Close()
    $stream.Close()
    Write-Host "  Packed $($allFiles.Count) files into $packFile"
}

Write-Host ""
Write-Host "Extracting icons ..."

foreach ($tool in $tools) {
    $folder = $tool.Folder
    $exeName = $tool.Exe
    $exePath = Join-Path $folder $exeName

    if (-not (Test-Path -LiteralPath $exePath)) {
        Write-Warning "  $exePath not found, skipping icon extraction."
        continue
    }

    $icoName = [System.IO.Path]::GetFileNameWithoutExtension($exeName) + ".ico"
    if ($exeName -eq "Core Temp.exe") {
        $icoName = "CoreTemp64.ico"
    }
    if ($exeName -eq "DiskMark64.exe") {
        $icoName = "DiskMark64.ico"
    }

    $icoPath = Join-Path $ResDir $icoName

    try {
        $icon = [System.Drawing.Icon]::ExtractAssociatedIcon($exePath)
        if ($null -ne $icon) {
            $fileStream = [System.IO.File]::Create($icoPath)
            $icon.Save($fileStream)
            $fileStream.Close()
            $icon.Dispose()
            Write-Host "  Extracted icon: $icoPath"
        }
        else {
            Write-Warning "  Failed to extract icon from $exePath"
        }
    }
    catch {
        Write-Warning "  Icon extraction error for $exePath : $_"
    }
}

Write-Host ""
Write-Host "Done."
