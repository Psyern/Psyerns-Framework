#Requires -Version 5.1
<#
.SYNOPSIS
    Liest alle Spieler-JSONs aus dem Ninjins Tracking Mod und überträgt
    sie an das WordPress Psyerns Framework Leaderboard Plugin.

.DESCRIPTION
    - Liest alle <STEAMID>.json Dateien aus dem Tracking-Mod Spielerordner
    - Mappt die Felder auf das WordPress API-Format (topPVEPlayers / topPVPPlayers)
    - Sendet via POST an /wp-json/psyern/v1/upload
    - Schreibt optional eine lokale leaderboard.json für die Standalone-Version

.NOTES
    Empfohlen: Via Windows Task Scheduler alle 10 Minuten ausführen.
    Log-Datei: neben dem Script als Send-LeaderboardToWordPress.log
#>

# ─────────────────────────────────────────────
#  KONFIGURATION — hier anpassen
# ─────────────────────────────────────────────

# Pfad zum Spielerordner des Ninjins Tracking Mods
$PlayersPath = "C:\Program Files (x86)\CFTools Software GmbH\Architect\Agent\deployments\DME_TestServer\profiles\Ninjins_Tracking_Mod\Data\Players"

# WordPress REST API URL (ohne trailing slash)
$WordPressUrl = "https://your-site.com/wp-json/psyern/v1/upload"

# API Key (aus WordPress Plugin → Psyerns Framework → Settings)
$ApiKey = "YOUR_API_KEY_HERE"

# Pfad zur lokalen standalone leaderboard.json (leer lassen um zu überspringen)
$StandaloneJsonPath = "C:\Users\Administrator\Desktop\Psyerns_Framework\MISC\standalone\data\leaderboard.json"

# Maximale Spieler pro Liste (PvE und PvP separat)
$TopLimit = 100

# HTTP Timeout in Sekunden
$TimeoutSec = 30

# ─────────────────────────────────────────────
#  LOGGING
# ─────────────────────────────────────────────

$LogFile = Join-Path $PSScriptRoot "Send-LeaderboardToWordPress.log"

function Write-Log {
    param([string]$Message, [string]$Level = "INFO")
    $timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    $line = "[$timestamp] [$Level] $Message"
    Add-Content -Path $LogFile -Value $line
    Write-Host $line
}

# ─────────────────────────────────────────────
#  SPIELER EINLESEN
# ─────────────────────────────────────────────

Write-Log "=== Leaderboard Sync gestartet ==="

if (-not (Test-Path $PlayersPath)) {
    Write-Log "Spielerordner nicht gefunden: $PlayersPath" "ERROR"
    exit 1
}

$playerFiles = Get-ChildItem -Path $PlayersPath -Filter "*.json" -ErrorAction SilentlyContinue
Write-Log "Gefundene Spieler-JSONs: $($playerFiles.Count)"

if ($playerFiles.Count -eq 0) {
    Write-Log "Keine Spieler-JSONs gefunden. Abbruch." "WARN"
    exit 0
}

$allPlayers = @()
$onlineCount = 0

foreach ($file in $playerFiles) {
    try {
        $raw = Get-Content $file.FullName -Raw -ErrorAction Stop
        $p   = $raw | ConvertFrom-Json

        # Steam ID aus Dateiname (Fallback auf PlayerID-Feld)
        $steamId = [System.IO.Path]::GetFileNameWithoutExtension($file.Name)
        if ($steamId -notmatch '^\d{17}$') {
            $steamId = "$($p.PlayerID)"
        }

        if ([string]::IsNullOrWhiteSpace($steamId)) {
            Write-Log "Kein SteamID in $($file.Name) — übersprungen" "WARN"
            continue
        }

        # Online-Status zählen
        if ([int]$p.playerIsOnline -eq 1) { $onlineCount++ }

        # Kills aus CategoryKills summieren
        $catKills   = $p.CategoryKills
        $totalKills = 0
        if ($catKills) {
            $catKills.PSObject.Properties | ForEach-Object { $totalKills += [int]$_.Value }
        }

        # Deaths aus CategoryDeaths summieren
        $catDeaths   = $p.CategoryDeaths
        $totalDeaths = 0
        if ($catDeaths) {
            $catDeaths.PSObject.Properties | ForEach-Object { $totalDeaths += [int]$_.Value }
        }

        # AI-Kills direkt aus CategoryKills
        $aiKills = 0
        if ($catKills -and $catKills.PSObject.Properties['AIBased']) {
            $aiKills = [int]$catKills.AIBased
        }

        # Längster Schuss = Maximum aus CategoryLongestRanges
        $catRanges   = $p.CategoryLongestRanges
        $longestShot = 0.0
        if ($catRanges) {
            $catRanges.PSObject.Properties | ForEach-Object {
                $v = [double]$_.Value
                if ($v -gt $longestShot) { $longestShot = $v }
            }
        }

        # War Boss Kills: kann Zahl oder Array sein
        $warBossKills = 0
        if ($p.WarBossKills -is [System.Array]) {
            $p.WarBossKills | ForEach-Object { $warBossKills += [int]$_ }
        } else {
            $warBossKills = [int]$p.WarBossKills
        }

        # Spielerobjekt bauen — Feldnamen exakt wie WordPress PHP erwartet
        $player = [ordered]@{
            playerID              = $steamId
            playerName            = "$($p.PlayerName)"
            pvePoints             = [int]$p.PvEPoints
            pvpPoints             = [int]$p.PvPPoints
            kills                 = $totalKills
            deaths                = $totalDeaths
            aiKills               = $aiKills
            longestShot           = [Math]::Round($longestShot, 2)
            playtime              = 0
            isOnline              = [int]$p.playerIsOnline
            lastLoginDate         = "$($p.LastLoginDate)"
            warFaction            = "$($p.WarFaction)"
            warAlignment          = [int]$p.WarAlignment
            warLevel              = [int]$p.WarLevel
            warBossKills          = $warBossKills
            hardlineReputation    = [int]$p.HardlineReputation
            categoryKills         = $catKills
            categoryDeaths        = $catDeaths
            categoryLongestRanges = $catRanges
        }

        $allPlayers += $player
    }
    catch {
        Write-Log "Fehler beim Lesen von $($file.Name): $_" "ERROR"
    }
}

Write-Log "Spieler eingelesen: $($allPlayers.Count) | Online: $onlineCount"

# ─────────────────────────────────────────────
#  SORTIEREN UND AUFTEILEN
# ─────────────────────────────────────────────

$topPVE = $allPlayers | Sort-Object { [int]$_.pvePoints } -Descending | Select-Object -First $TopLimit
$topPVP = $allPlayers | Sort-Object { [int]$_.pvpPoints } -Descending | Select-Object -First $TopLimit

# Faction-Punkte summieren (EAST = Factions mit "EAST" oder "west"/"WEST" für WEST)
$globalEastPoints = ($allPlayers | Where-Object { $_.warFaction -eq "EAST" } | Measure-Object -Property pvePoints -Sum).Sum
$globalWestPoints = ($allPlayers | Where-Object { $_.warFaction -eq "WEST" } | Measure-Object -Property pvePoints -Sum).Sum
if (-not $globalEastPoints) { $globalEastPoints = 0 }
if (-not $globalWestPoints) { $globalWestPoints = 0 }

# ─────────────────────────────────────────────
#  PAYLOAD BAUEN
# ─────────────────────────────────────────────

$payload = [ordered]@{
    apiKey               = $ApiKey
    generatedAt          = ([System.DateTime]::UtcNow.ToString("yyyy-MM-ddTHH:mm:ssZ"))
    playerOnlineCounter  = $onlineCount
    totalPlayers         = $allPlayers.Count
    globalEastPoints     = [int]$globalEastPoints
    globalWestPoints     = [int]$globalWestPoints
    topPVEPlayers        = @($topPVE)
    topPVPPlayers        = @($topPVP)
}

$json = $payload | ConvertTo-Json -Depth 10 -Compress

# ─────────────────────────────────────────────
#  AN WORDPRESS SENDEN
# ─────────────────────────────────────────────

Write-Log "Sende an WordPress: $WordPressUrl"

try {
    $headers = @{
        "Content-Type" = "application/json"
        "X-API-Key"    = $ApiKey
    }

    $response = Invoke-RestMethod `
        -Uri $WordPressUrl `
        -Method POST `
        -Body $json `
        -Headers $headers `
        -TimeoutSec $TimeoutSec `
        -ErrorAction Stop

    if ($response.success -eq $true) {
        Write-Log "WordPress: Upload erfolgreich ($($allPlayers.Count) Spieler)"
    } else {
        Write-Log "WordPress: Unerwartete Antwort: $($response | ConvertTo-Json -Compress)" "WARN"
    }
}
catch {
    $statusCode = $_.Exception.Response.StatusCode.value__
    Write-Log "WordPress Upload fehlgeschlagen (HTTP $statusCode): $_" "ERROR"
}

# ─────────────────────────────────────────────
#  LOKALE STANDALONE JSON SCHREIBEN (optional)
# ─────────────────────────────────────────────

if (-not [string]::IsNullOrWhiteSpace($StandaloneJsonPath)) {
    try {
        $standalonePayload = [ordered]@{
            generatedAt         = $payload.generatedAt
            playerOnlineCounter = $payload.playerOnlineCounter
            totalPlayers        = $payload.totalPlayers
            globalEastPoints    = $payload.globalEastPoints
            globalWestPoints    = $payload.globalWestPoints
            topPVEPlayers       = @($topPVE)
            topPVPPlayers       = @($topPVP)
        }

        $standalonePayload | ConvertTo-Json -Depth 10 | Set-Content -Path $StandaloneJsonPath -Encoding UTF8
        Write-Log "Standalone JSON geschrieben: $StandaloneJsonPath"
    }
    catch {
        Write-Log "Fehler beim Schreiben der Standalone JSON: $_" "ERROR"
    }
}

Write-Log "=== Leaderboard Sync abgeschlossen ==="
