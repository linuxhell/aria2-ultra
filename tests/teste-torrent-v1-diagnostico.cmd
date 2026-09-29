@echo off
setlocal DisableDelayedExpansion
set "BASE=%~dp0..\"
if not exist "%BASE%aria2c.exe" (
  echo Falta aria2c.exe na raiz do pacote.
  echo Baixe o binario Windows x64 na pagina de Releases do repositorio
  echo e coloque como "%BASE%aria2c.exe".
  exit /b 1
)

set "STAMP=%RANDOM%-%RANDOM%"
set "LOG=%BASE%aria2-ultra-torrent-v1-diagnostico-%STAMP%.log"
set "DEST=%BASE%teste-torrent-v1-diagnostico-%STAMP%"
mkdir "%DEST%" || exit /b 1

set /p "TORRENT=Cole um magnet v1 ou caminho de arquivo .torrent e pressione Enter: "
if not defined TORRENT (
  echo Nenhum torrent informado.
  exit /b 1
)

echo.
echo TESTE TORRENT V1 - DIAGNOSTICO/PEERS
echo Log: "%LOG%"
echo Destino: "%DEST%"
echo.
echo Este teste NAO altera o teste de download direto.
echo Ele aumenta a busca por peers e reduz espera em trackers mortos.
echo Ele tambem prioriza inicio/fim dos arquivos para permitir pre-visualizacao no VLC durante o download.
echo Pressione Ctrl+C depois de alguns minutos se quiser apenas medir velocidade.
echo.

"%BASE%aria2c.exe" ^
  --no-conf=true ^
  --file-allocation=none ^
  --seed-time=0 ^
  --enable-dht=true ^
  --enable-dht6=true ^
  --enable-peer-exchange=true ^
  --bt-enable-lpd=true ^
  --bt-max-peers=200 ^
  --bt-request-peer-speed-limit=5M ^
  --bt-tracker-connect-timeout=10 ^
  --bt-tracker-timeout=15 ^
  --bt-save-metadata=true ^
  --bt-prioritize-piece=head=10M,tail=1M ^
  --log="%LOG%" ^
  --log-level=info ^
  --console-log-level=notice ^
  --summary-interval=1 ^
  --dir="%DEST%" ^
  "%TORRENT%"

set "RESULT=%ERRORLEVEL%"
echo.
echo Codigo de saida: %RESULT%
echo Envie o log: "%LOG%"
pause
exit /b %RESULT%
