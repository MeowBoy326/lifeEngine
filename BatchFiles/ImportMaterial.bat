@echo off
setlocal

cd ../Binaries/Win64/
HeliumGame-Win64-Debug.exe ImportMaterial -src "%1" -pak "%2" %*