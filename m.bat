@echo off

REM del *.obj
del d4.exe
del sb.exe

wmake

c:\dos32a\binw\sb /b /BNsb.exe d4.exe