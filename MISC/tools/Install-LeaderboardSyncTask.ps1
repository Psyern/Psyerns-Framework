#Requires -RunAsAdministrator
<#
.SYNOPSIS
    Registriert Send-LeaderboardToWordPress.ps1 als Windows Task Scheduler Job.

.DESCRIPTION
    - Task läuft alle 10 Minuten
    - Startet auch wenn kein Benutzer angemeldet ist
    - Loggt Ergebnisse in Send-LeaderboardToWordPress.log neben dem Script

.NOTES
    Muss als Administrator ausgeführt werden.
    PowerShell: Right-Click → "Als Administrator ausführen"
#>

$ScriptPath  = "$PSScriptRoot\Send-LeaderboardToWordPress.ps1"
$TaskName    = "PsyernsFramework_LeaderboardSync"
$Description = "Überträgt Ninjins Tracking Mod Spielerdaten an das WordPress Psyerns Framework Leaderboard"

# Prüfen ob Script existiert
if (-not (Test-Path $ScriptPath)) {
    Write-Host "[FEHLER] Script nicht gefunden: $ScriptPath" -ForegroundColor Red
    Write-Host "Stelle sicher dass Send-LeaderboardToWordPress.ps1 im gleichen Ordner liegt." -ForegroundColor Yellow
    exit 1
}

# Bestehenden Task entfernen falls vorhanden
$existing = Get-ScheduledTask -TaskName $TaskName -ErrorAction SilentlyContinue
if ($existing) {
    Unregister-ScheduledTask -TaskName $TaskName -Confirm:$false
    Write-Host "[INFO] Bestehender Task '$TaskName' entfernt."
}

# Action: PowerShell Script ausführen
$action = New-ScheduledTaskAction `
    -Execute "powershell.exe" `
    -Argument "-NonInteractive -NoProfile -ExecutionPolicy Bypass -File `"$ScriptPath`""

# Trigger: Alle 10 Minuten, dauerhaft
$trigger = New-ScheduledTaskTrigger -RepetitionInterval (New-TimeSpan -Minutes 10) -Once -At (Get-Date)

# Settings
$settings = New-ScheduledTaskSettingsSet `
    -ExecutionTimeLimit (New-TimeSpan -Minutes 5) `
    -RestartCount 2 `
    -RestartInterval (New-TimeSpan -Minutes 1) `
    -StartWhenAvailable `
    -RunOnlyIfNetworkAvailable

# Principal: SYSTEM — läuft auch ohne Benutzeranmeldung
$principal = New-ScheduledTaskPrincipal -UserId "SYSTEM" -LogonType ServiceAccount -RunLevel Highest

# Task registrieren
Register-ScheduledTask `
    -TaskName    $TaskName `
    -Description $Description `
    -Action      $action `
    -Trigger     $trigger `
    -Settings    $settings `
    -Principal   $principal `
    -Force | Out-Null

Write-Host ""
Write-Host "✓ Task Scheduler Job registriert:" -ForegroundColor Green
Write-Host "  Name:      $TaskName"
Write-Host "  Script:    $ScriptPath"
Write-Host "  Interval:  alle 10 Minuten"
Write-Host "  Benutzer:  SYSTEM (läuft ohne Anmeldung)"
Write-Host ""
Write-Host "Zum Testen jetzt sofort ausführen:" -ForegroundColor Cyan
Write-Host "  Start-ScheduledTask -TaskName '$TaskName'"
Write-Host ""
Write-Host "Task entfernen:" -ForegroundColor Yellow
Write-Host "  Unregister-ScheduledTask -TaskName '$TaskName' -Confirm:`$false"
