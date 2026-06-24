$maxBatchSizeBytes = 1500000000 # 1.5 GB in bytes. Safe ceiling to stay well under GitHub's 2GB cutoff
Write-Host "Starting dynamic batch push. We will push up to 1.5 GB per commit."

# Dynamically calculate the next batch number by checking Git history
$maxBatch = 0
git log --format="%s" -n 200 | ForEach-Object {
    if ($_ -match "Add Project file part (\d+)") {
        $num = [int]$matches[1]
        if ($num -gt $maxBatch) { $maxBatch = $num }
    }
}
$batchNum = $maxBatch + 1

while ($true) {
    # Get all untracked files
    $files = git ls-files --others --exclude-standard
    if ($null -eq $files -or $files.Count -eq 0) {
        Write-Host "No more untracked files found. Done!"
        break
    }

    if ($files -isnot [array]) {
        $files = @($files)
    }

    $batch = @()
    $currentBatchSizeBytes = 0
    $totalLeft = $files.Count

    # Find completely safe chunk of files that totals strictly under our Max GB Limit
    foreach ($file in $files) {
        $size = 0
        if (Test-Path -LiteralPath $file) {
            $size = (Get-Item -LiteralPath $file).Length
        }
        
        # Stop building the batch if the NEXT file will push us over the threshold.
        # (Using -gt 0 ensures we always push at least 1 file, even if it is massively huge)
        if ($batch.Count -gt 0 -and ($currentBatchSizeBytes + $size) -ge $maxBatchSizeBytes) {
            break
        }
        
        $batch += $file
        $currentBatchSizeBytes += $size
    }

    $batchCount = $batch.Count
    $sizeMB = [math]::Round($currentBatchSizeBytes / 1MB, 2)
    
    Write-Host "Files remaining: $totalLeft"
    Write-Host "Adding a carefully sized batch of $batchCount files ($sizeMB MB)..."
    
    foreach ($file in $batch) {
        git add $file
    }
    
    Write-Host "Committing as Part $batchNum..."
    git commit -m "Add Project file part $batchNum"
    
    Write-Host "Pushing to GitHub..."
    git push origin master
    
    if ($LASTEXITCODE -ne 0) {
        Write-Host "An error occurred during push. The script will stop so you can investigate."
        break
    }
    
    Write-Host "Batch $batchNum finished successfully!`n"
    $batchNum++
}
