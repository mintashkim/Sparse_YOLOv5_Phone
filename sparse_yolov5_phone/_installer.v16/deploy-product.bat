rmdir .\_dst /S /Q

Set SignServer=192.168.1.56

..\..\Common.v16\Common2\bin\x64\build_tool.exe /deploy deploy\files.ini ALL
if not "%ERRORLEVEL%" == "0" goto ERROR

..\..\Common.v16\Common2\bin\x64\build_tool.exe /version -f _bin\bin_x64\abgConsole.exe -i _inst\deploy\info_.xml -o _inst\info.xml
if not "%ERRORLEVEL%" == "1" goto ERROR

"%PROGRAMFILES%\NSIS\Unicode\makensis.exe" /DWRITE_UNINST product-setup.nsi
if not "%ERRORLEVEL%" == "0" goto ERROR
"_bin\_writer-uninst.exe"
if "%ERRORLEVEL%" == "0" goto ERROR
..\..\Common.v16\Common\bin\RemoteSignTool %SignServer% SHA2EV _bin\uninst.exe || goto ERROR
copy _bin\uninst.exe _bin\bin_x64\
::copy _bin\uninst.exe _bin\bin_x86\

::"%PROGRAMFILES%\NSIS\Unicode\makensis.exe" /DWRITE_UNINST product-setup-dbg.nsi
::if not "%ERRORLEVEL%" == "0" goto ERROR
::"_dbg\_writer-uninst.exe"
::if "%ERRORLEVEL%" == "0" goto ERROR
::..\..\Common.v16\Common\bin\RemoteSignTool %SignServer% SHA2EV _dbg\uninst.exe || goto ERROR
::copy _dbg\uninst.exe _dbg\bin_x64\
::copy _dbg\uninst.exe _dbg\bin_x86\
	
mkdir .\_dst\bin
"%PROGRAMFILES%\NSIS\Unicode\makensis.exe" "/XOutFile bossgo-setup.exe" product-setup.nsi
if not "%ERRORLEVEL%" == "0" goto ERROR
..\..\Common.v16\Common\bin\RemoteSignTool %SignServer% SHA2EV _dst\bin\bossgo-setup.exe || goto ERROR

mkdir .\_dst\pdb
"%PROGRAMFILES%\7-zip\7z.exe" a -tzip _dst\pdb\bossgo-pdb.zip _pdb
if not "%ERRORLEVEL%" == "0" goto ERROR

mkdir .\_dst\bin\LogViewer
::..\..\Common.v16\Common\bin\ETViewer.exe /TS:.\_dst\bin\LogViewer\ADULogViewer_x86.exe _pdb\rel.x86\*.pdb
..\..\Common.v16\Common\bin\ETViewer.exe /TS:.\_dst\bin\LogViewer\ABGLogViewer_x64.exe _pdb\rel.x64\*.pdb
::..\..\Common.v16\Common\bin\ETViewer.exe /TS:.\_dst\bin\LogViewer\ADULogViewer_x86_dbg.exe _pdb\dbg.x86\*.pdb
::..\..\Common.v16\Common\bin\ETViewer.exe /TS:.\_dst\bin\LogViewer\ADULogViewer_x64_dbg.exe _pdb\dbg.x64\*.pdb
::..\..\Common.v16\Common\bin\RemoteSignTool %SignServer% SHA2EV _dst\bin\LogViewer\ADULogViewer_x86.exe || goto ERROR
..\..\Common.v16\Common\bin\RemoteSignTool %SignServer% SHA2EV _dst\bin\LogViewer\ABGLogViewer_x64.exe || goto ERROR
::..\..\Common.v16\Common\bin\RemoteSignTool %SignServer% SHA2EV _dst\bin\LogViewer\ADULogViewer_x86_dbg.exe || goto ERROR
::..\..\Common.v16\Common\bin\RemoteSignTool %SignServer% SHA2EV _dst\bin\LogViewer\ADULogViewer_x64_dbg.exe || goto ERROR

goto QUIT

:ERROR
::pause
exit 1

:QUIT