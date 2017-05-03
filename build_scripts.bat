@echo off

@set PATH=%PATH%;E:\Steam\steamapps\common\Skyrim\Papyrus Compiler
@set LIBS=E:\dev\projects\Skyrim\Libraries\Papyrus

@set PPF_FLAGS=E:\Steam\steamapps\common\Skyrim\TESV_Papyrus_Flags.flg
@set PPF_SCRIPTS=%LIBS%\SKSE;%LIBS%\Skyrim;%LIBS%\SkyUI 5.1;%LIBS%\SLSF 0_99;%LIBS%\PapyrusUtil 3_3_0_0;%LIBS%\SEXLAB_1_62;%LIBS%\FNIS 6_3;%LIBS%\FNIS Creature Pack 6_1;%LIBS%\RaceMenu 3_4_5;SkyUtilities\Papyrus Exports

@mkdir "Release\Scripts" 2>nul
@del /Q /F "Release\Scripts\*" 2>nul

for %%i in ("SkyUtilities\Papyrus Exports\*") do (
	@echo PapyrusCompiler "%%i" -i="%PPF_SCRIPTS%" -o="Release\Scripts" -f="%PPF_FLAGS%"
	PapyrusCompiler "%%i" -i="%PPF_SCRIPTS%" -o="Release\Scripts" -f="%PPF_FLAGS%"

	if not ERRORLEVEL 0 goto end
)

@xcopy /Y "SkyUtilities\Papyrus Exports\*.psc" %LIBS%\SkyUtilities

:end
@pause