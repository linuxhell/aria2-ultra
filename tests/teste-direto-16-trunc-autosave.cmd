@echo off
rem aria2-ultra baseline throughput regression test
setlocal DisableDelayedExpansion
set "BASE=%~dp0..\"
if not exist "%BASE%aria2c.exe" (
  echo Falta aria2c.exe na raiz do pacote.
  exit /b 1
)
set "LOG=%BASE%aria2-ultra-direto-16-trunc-autosave-%RANDOM%-%RANDOM%.log"
set "DEST=%BASE%teste-direto-16-trunc-autosave-%RANDOM%-%RANDOM%"
mkdir "%DEST%" || exit /b 1
set "ISO_URL=%~1"
if not defined ISO_URL set /p "ISO_URL=Cole o link completo e valido da ISO Microsoft e pressione Enter: "
if not defined ISO_URL (
  echo Nenhum link informado.
  exit /b 1
)
echo TESTE DE RETOMADA: salvamento periodico a cada 60 segundos.
echo TESTE DE DOWNLOAD DIRETO - 16 CONEXOES - TRUNC
echo Log: "%LOG%"
echo Destino: "%DEST%"
> "%LOG%" echo ULTRA TEST: launcher=teste-direto-16-trunc-autosave.cmd split=16 max_connection_per_server=16 min_split_size=1M file_allocation=trunc auto_save_interval=60 no_conf=true
>> "%LOG%" echo ULTRA TEST: begin=%DATE% %TIME%
"%BASE%aria2c.exe" --version >> "%LOG%" 2>&1
"%BASE%aria2c.exe" --no-conf=true --file-allocation=trunc --auto-save-interval=60 --continue=false --auto-file-renaming=false --split=16 --max-connection-per-server=16 --min-split-size=1M --log="%LOG%" --log-level=notice --console-log-level=warn --summary-interval=1 --dir="%DEST%" "%ISO_URL%"
set "RESULT=%ERRORLEVEL%"
>> "%LOG%" echo ULTRA TEST: end=%DATE% %TIME% exit_code=%RESULT%
echo Codigo de saida: %RESULT%
echo Envie o log: "%LOG%"
pause
exit /b %RESULT%
