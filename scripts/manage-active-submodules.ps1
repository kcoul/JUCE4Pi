param(
    [switch]$Apply,
    [string]$DefaultBranch = "main"
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

function Get-RepoRoot {
    (git rev-parse --show-toplevel).Trim()
}

function Parse-HostOwner {
    param([string]$Url)

    # ssh: git@github.com:owner/repo.git
    if ($Url -match '^[^@]+@([^:]+):([^/]+)/.+$') {
        return @{ Host = $Matches[1]; Owner = $Matches[2] }
    }

    # scheme: https://host/owner/repo.git
    if ($Url -match '^[a-zA-Z][a-zA-Z0-9+.-]*://([^/]+)/([^/]+)/.+$') {
        return @{ Host = $Matches[1]; Owner = $Matches[2] }
    }

    # scp-like without user
    if ($Url -match '^([^:]+):([^/]+)/.+$') {
        return @{ Host = $Matches[1]; Owner = $Matches[2] }
    }

    return $null
}

$repoRoot = Get-RepoRoot
Set-Location $repoRoot

if (-not (Test-Path ".gitmodules")) {
    Write-Host "No .gitmodules found in $repoRoot"
    exit 0
}

$rootUrl = (git remote get-url origin).Trim()
$root = Parse-HostOwner -Url $rootUrl
if ($null -eq $root) {
    throw "Could not parse root origin URL: $rootUrl"
}

Write-Host "Root origin: $rootUrl"
Write-Host "Root host/owner: $($root.Host) / $($root.Owner)"
Write-Host ""
Write-Host ('{0,-10}  {1,-45}  {2,-24}  {3}' -f 'Class', 'Submodule', 'Owner', 'URL')
Write-Host ('{0,-10}  {1,-45}  {2,-24}  {3}' -f '----------', '---------------------------------------------', '------------------------', '------------------------------')

$paths = @(git submodule status --recursive |
    ForEach-Object { ($_ -split "\s+")[1] } |
    Where-Object { $_ -and $_.Trim().Length -gt 0 })

$activePaths = New-Object System.Collections.Generic.List[string]

foreach ($path in $paths) {
    if (-not (Test-Path $path)) {
        Write-Host ('{0,-10}  {1,-45}  {2,-24}  {3}' -f 'missing', $path, '-', '(not initialized)')
        continue
    }

    $smUrl = (git -C $path remote get-url origin 2>$null)
    if ([string]::IsNullOrWhiteSpace($smUrl)) {
        Write-Host ('{0,-10}  {1,-45}  {2,-24}  {3}' -f 'unknown', $path, '-', '(no origin)')
        continue
    }

    $sm = Parse-HostOwner -Url $smUrl
    if ($null -eq $sm) {
        Write-Host ('{0,-10}  {1,-45}  {2,-24}  {3}' -f 'unknown', $path, '-', $smUrl)
        continue
    }

    $class = 'dormant'
    if ($sm.Host -eq $root.Host -and $sm.Owner -eq $root.Owner) {
        $class = 'active'
        $activePaths.Add($path)
    }

    $ownerLabel = "$($sm.Owner)@$($sm.Host)"
    Write-Host ('{0,-10}  {1,-45}  {2,-24}  {3}' -f $class, $path, $ownerLabel, $smUrl)
}

if (-not $Apply) {
    Write-Host ""
    Write-Host "Dry run only. Re-run with -Apply to configure active submodules."
    exit 0
}

Write-Host ""
foreach ($path in $activePaths) {
    Write-Host "[apply] $path"

    $currentBranch = (git -C $path symbolic-ref -q --short HEAD 2>$null)
    $targetBranch = $null

    if (-not [string]::IsNullOrWhiteSpace($currentBranch)) {
        $targetBranch = $currentBranch.Trim()
    } else {
        $remoteHead = (git -C $path symbolic-ref -q --short refs/remotes/origin/HEAD 2>$null)
        if (-not [string]::IsNullOrWhiteSpace($remoteHead)) {
            $targetBranch = $remoteHead.Trim() -replace '^origin/', ''
        } else {
            $targetBranch = $DefaultBranch
        }
    }

    $dirty = (git -C $path status --porcelain)
    if ($dirty) {
        Write-Host "  [warn] dirty tree, skipping branch attach"
    } else {
        if ($currentBranch -and $currentBranch.Trim() -eq $targetBranch) {
            Write-Host "  [ok] already on branch $targetBranch"
        } else {
            git -C $path fetch --quiet origin $targetBranch 2>$null

            git -C $path show-ref --verify --quiet "refs/heads/$targetBranch"
            if ($LASTEXITCODE -eq 0) {
                git -C $path switch $targetBranch | Out-Null
                Write-Host "  [fix] switched to local branch $targetBranch"
            } else {
                git -C $path show-ref --verify --quiet "refs/remotes/origin/$targetBranch"
                if ($LASTEXITCODE -eq 0) {
                    git -C $path switch -c $targetBranch --track "origin/$targetBranch" | Out-Null
                    Write-Host "  [fix] created tracking branch $targetBranch"
                } else {
                    Write-Host "  [warn] cannot find branch $targetBranch locally or on origin"
                }
            }
        }
    }

    $hooksDir = (git -C $path rev-parse --git-path hooks).Trim()
    New-Item -ItemType Directory -Path $hooksDir -Force | Out-Null

    $hookPath = Join-Path $hooksDir "pre-commit"
    $hook = @'
#!/usr/bin/env bash
set -euo pipefail

if ! git symbolic-ref -q HEAD >/dev/null; then
  echo "Refusing commit: detached HEAD. Switch to a branch first." >&2
  exit 1
fi
'@
    Set-Content -Path $hookPath -Value $hook -NoNewline

    Write-Host "  [fix] installed detached-HEAD pre-commit guard"
}

Write-Host ""
Write-Host "Done."
