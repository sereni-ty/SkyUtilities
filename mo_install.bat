@echo off

@set MO_MOD_PATH=E:\Mod Organizer\mods\SkyUtilities
@set PROJECT_PATH=E:\dev\projects\Skyrim\SKSE\SkyUtilities\SkyUtilities
@set SKYRIM_PLUGIN_OUT_PATH=E:\dev\projects\Skyrim\SKSE\SkyUtilities\SkyUtilities\%1\Skyrim Plugin

@cmd /C %PROJECT_PATH%\build.bat %1

@rmdir /S /Q "%MO_MOD_PATH%\"
@mkdir "%MO_MOD_PATH%"
@xcopy /Y /S "%SKYRIM_PLUGIN_OUT_PATH%\*" "%MO_MOD_PATH%"