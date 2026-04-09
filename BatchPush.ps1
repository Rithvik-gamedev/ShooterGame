$batchSize = 80
Write-Host "Starting batch push. We will add and push $batchSize files at a time."

$batchNum = 1

while ($true) {
    # Get all untracked files
    $files = git ls-files --others --exclude-standard
    if ($null -eq $files -or $files.Count -eq 0) {
        Write-Host "No more untracked files found. Done!"
        break
    }

    # Ensure $files is treated as an array even if there's only 1 file
    if ($files -isnot [array]) {
        $files = @($files)
    }

    $batch = $files | Select-Object -First $batchSize
    if ($batch -isnot [array]) {
        $batch = @($batch)
    }

    $batchCount = $batch.Count
    $totalLeft = $files.Count
    
    Write-Host "Files remaining: $totalLeft"
    Write-Host "Adding a batch of $batchCount files..."
    
    foreach ($file in $batch) {
        git add $file
    }
    
    Write-Host "Committing..."
    git commit -m "Add Project file part $batchNum"
    
    Write-Host "Pushing to GitHub..."
    git push origin master
    
    if ($LASTEXITCODE -ne 0) {
        Write-Host "An error occurred during push. The script will stop so you can investigate."
        break
    }
    
    Write-Host "Batch finished successfully!`n"
    $batchNum++
}
