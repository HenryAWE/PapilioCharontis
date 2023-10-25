Push-Location

New-Item -ItemType Directory -Path "build" -Force
Set-Location .\build
cmake -GNinja $args ..
cmake --build .

Pop-Location
