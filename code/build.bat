@echo off

set CommonCompilerFlags=-MTd -nologo -fp:fast -Gm- -GR- -EHa- -Od -Oi -WX -W4 -wd4201 -wd4100 -wd4127 -wd4189 -wd4505 -FC -Z7
set CommonLinkerFlags= -incremental:no -opt:ref user32.lib gdi32.lib winmm.lib



call "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" x64



pushd ..\build

REM 32-bit build
REM cl %CommonCompilerFlags% ..\code\win32_carcassone.cpp /link -subsystem,5.1 %CommonLinkerFlags%

REM 64-bit build
del *.pdb > NUL 2> NUL
echo WAITING FOR PDB > lock.tmp
cl %CommonCompilerFlags% ..\code\carcassone.cpp -LD /link -incremental:no -opt:ref 
del lock.tmp
cl %CommonCompilerFlags% ..\code\win32_carcassone.cpp /link %CommonLinkerFlags%
popd