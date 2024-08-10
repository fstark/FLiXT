@echo off
del flixt.exe
echo Assembling
tcc -c demo.asm
if errorlevel 1 goto error
tc /b
echo Done
:error

