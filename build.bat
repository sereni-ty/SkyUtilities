@echo off

@set PATH=%PATH%;E:\Steam\steamapps\common\Skyrim\Papyrus Compiler

@set PROJECT_PATH=E:\dev\projects\Skyrim\SKSE\SkyUtilities\SkyUtilities
@set SKYRIM_PLUGIN_SRC_PATH=E:\dev\projects\Skyrim\SKSE\SkyUtilities\SkyUtilities\SkyUtilities\Skyrim Plugin
@set SKYRIM_PLUGIN_OUT_PATH=E:\dev\projects\Skyrim\SKSE\SkyUtilities\SkyUtilities\%1\Skyrim Plugin

@set LIBS=E:\dev\projects\Skyrim\Libraries\Papyrus
@set PPF_FLAGS=E:\Steam\steamapps\common\Skyrim\TESV_Papyrus_Flags.flg
@set PPF_SCRIPTS=%LIBS%\SKSE;%LIBS%\Skyrim;%LIBS%\SkyUI 5.1;%LIBS%\SLSF 0_99;%LIBS%\PapyrusUtil 3_3_0_0;%LIBS%\SEXLAB_1_62;%LIBS%\FNIS 6_3;%LIBS%\FNIS Creature Pack 6_1;%LIBS%\RaceMenu 3_4_5;%SKYRIM_PLUGIN_SRC_PATH%\Scripts\Source

@rmdir /S /Q "%SKYRIM_PLUGIN_OUT_PATH%"
@mkdir "%SKYRIM_PLUGIN_OUT_PATH%"
@mkdir "%SKYRIM_PLUGIN_OUT_PATH%\SKSE"
@mkdir "%SKYRIM_PLUGIN_OUT_PATH%\SKSE\Plugins"

@xcopy /S /Y "%SKYRIM_PLUGIN_SRC_PATH%\*" "%SKYRIM_PLUGIN_OUT_PATH%"
@copy /Y "%PROJECT_PATH%\%1\SkyUtilities.dll" "%SKYRIM_PLUGIN_OUT_PATH%\SKSE\Plugins\SkyUtilities.dll" 

@xcopy /Y "%SKYRIM_PLUGIN_SRC_PATH%\Scripts\Source\*.psc" "%LIBS%\SkyUtilities"

for %%i in ("%SKYRIM_PLUGIN_OUT_PATH%\Scripts\Source\*") do (
	@echo PapyrusCompiler "%%i" -i="%PPF_SCRIPTS%" -o="%SKYRIM_PLUGIN_OUT_PATH%\Scripts" -f="%PPF_FLAGS%"
	PapyrusCompiler "%%i" -i="%PPF_SCRIPTS%" -o="%SKYRIM_PLUGIN_OUT_PATH%\Scripts" -f="%PPF_FLAGS%"

	if not ERRORLEVEL 0 goto end
)



:end