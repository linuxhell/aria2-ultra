@echo off
setlocal DisableDelayedExpansion
set "BASE=%~dp0"
if not exist "%BASE%aria2c.exe" (
  echo Falta aria2c.exe na mesma pasta deste arquivo.
  exit /b 1
)
set "LOG=%BASE%aria2-ultra-torrent-v2-%RANDOM%.log"
set "DEST=%BASE%teste-torrent-%RANDOM%"
mkdir "%DEST%" || exit /b 1
set /p "TORRENT=Cole um magnet ou caminho de arquivo .torrent e pressione Enter: "
if not defined TORRENT (
  echo Nenhum torrent informado.
  exit /b 1
)
if /i "%TORRENT:~0,7%"=="http://" goto HTTP_INPUT
if /i "%TORRENT:~0,8%"=="https://" goto HTTP_INPUT
echo Log de diagnostico: "%LOG%"
echo Arquivos do torrent: "%DEST%"
"%BASE%aria2c.exe" --no-conf=true --file-allocation=none --seed-time=0 --log="%LOG%" --log-level=notice --console-log-level=warn --summary-interval=1 --dir="%DEST%" "%TORRENT%"
set "RESULT=%ERRORLEVEL%"
echo.
echo Codigo de saida: %RESULT%
echo Envie o log: "%LOG%"
exit /b %RESULT%
:HTTP_INPUT
echo Voce informou um link HTTP. Para baixar ISO, execute teste-iso.cmd.
echo Para testar torrent, cole um magnet ou o caminho de um arquivo .torrent.
exit /b 2
