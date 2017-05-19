@echo off

@set MO_MOD_PATH=E:\Mod Organizer\mods\SkyUtilities
@set PROJECT_PATH=E:\dev\projects\Skyrim\SKSE\SkyUtilities\SkyUtilities

@cmd /C %PROJECT_PATH%\build_scripts.bat %1

@xcopy /Y "%1SkyUtilities.dll" "%MO_MOD_PATH%\SKSE\Plugins\SkyUtilities.dll" 2> nul
@xcopy /Y "%1Scripts" "%MO_MOD_PATH%\Scripts" 2> nul