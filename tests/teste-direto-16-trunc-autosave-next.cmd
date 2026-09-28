@echo off
rem aria2-ultra: mesma regressao de throughput do teste classico (split=16,
rem trunc, auto-save=60s), portada para o motor aria2-next (libcurl + estado
rem proprio em --state-dir). NAO mexe em teste-direto-16-trunc-autosave.cmd
rem nem em aria2c.exe: aquele teste continua validando o binario antigo.
setlocal DisableDelayedExpansion
set "BASE=%~dp0..\"
if not exist "%BASE%aria2-next.exe" (
  echo Falta aria2-next.exe na raiz do pacote.
  echo Baixe o binario Windows x64 em https://github.com/AnInsomniacy/aria2-next/releases
  echo e coloque como "%BASE%aria2-next.exe".
  exit /b 1
)
set "LOG=%BASE%aria2-ultra-direto-16-trunc-autosave-next-%RANDOM%-%RANDOM%.log"
set "DEST=%BASE%teste-direto-16-trunc-autosave-next-%RANDOM%-%RANDOM%"
mkdir "%DEST%" || exit /b 1
set "ISO_URL=%~1"
if not defined ISO_URL set /p "ISO_URL=Cole o link completo e valido da ISO Microsoft e pressione Enter: "
if not defined ISO_URL (
  echo Nenhum link informado.
  exit /b 1
)
echo TESTE DE RETOMADA: salvamento periodico a cada 60 segundos.
echo TESTE DE DOWNLOAD DIRETO - 16 CONEXOES - TRUNC (aria2-next)
echo Log: "%LOG%"
echo Destino: "%DEST%"
rem Traducao explicita das flags classicas para os nomes nativos do
rem aria2-next, equivalente ao que o adaptador de compatibilidade dele
rem faria sozinho (split + max-connection-per-server => stream-max-connections;
rem auto-save-interval => state-save-interval). min-split-size nao tem
rem equivalente nativo: o aria2-next decide o tamanho de faixa HTTP
rem automaticamente (stream-max-range-size=0), sem prejuizo ao numero
rem de conexoes pedido.
> "%LOG%" echo ULTRA TEST: launcher=teste-direto-16-trunc-autosave-next.cmd stream_max_connections=16 file_allocation=trunc state_save_interval=60 no_conf=true
>> "%LOG%" echo ULTRA TEST: begin=%DATE% %TIME%
"%BASE%aria2-next.exe" --version >> "%LOG%" 2>&1
"%BASE%aria2-next.exe" --no-conf=true --file-allocation=trunc --state-save-interval=60 --continue=false --auto-file-renaming=false --stream-max-connections=16 --state-dir="%DEST%\state" --log="%LOG%" --log-level=info --console-log-level=warn --summary-interval=1 --dir="%DEST%" "%ISO_URL%"
set "RESULT=%ERRORLEVEL%"
>> "%LOG%" echo ULTRA TEST: end=%DATE% %TIME% exit_code=%RESULT%
echo Codigo de saida: %RESULT%
echo Envie o log: "%LOG%"
pause
exit /b %RESULT%
