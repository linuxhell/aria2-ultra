@echo off
rem aria2-ultra: diagnostico de torrent (v1, v2 e hibrido) no motor
rem aria2-next, que usa libtorrent-rasterbar 2.1 nativamente. Ao contrario
rem do aria2c.exe classico (so v1), este binario aceita o mesmo magnet ou
rem .torrent seja ele v1, v2 (BEP 52) ou hibrido, sem flag especial:
rem libtorrent detecta o formato sozinho pelo info-hash/metainfo.
rem
rem Isto NAO altera teste-torrent-v1-diagnostico.cmd (aria2c.exe classico)
rem nem os testes de download direto.
setlocal DisableDelayedExpansion
set "BASE=%~dp0"
if not exist "%BASE%aria2-next.exe" (
  echo Falta aria2-next.exe na mesma pasta deste arquivo.
  echo Baixe o binario Windows x64 em https://github.com/AnInsomniacy/aria2-next/releases
  echo e coloque como "%BASE%aria2-next.exe".
  exit /b 1
)

set "STAMP=%RANDOM%-%RANDOM%"
set "LOG=%BASE%aria2-ultra-torrent-diagnostico-next-%STAMP%.log"
set "DEST=%BASE%teste-torrent-diagnostico-next-%STAMP%"
mkdir "%DEST%" || exit /b 1

set /p "TORRENT=Cole um magnet (v1 ou v2) ou caminho de arquivo .torrent e pressione Enter: "
if not defined TORRENT (
  echo Nenhum torrent informado.
  exit /b 1
)

echo.
echo TESTE TORRENT V1/V2 - DIAGNOSTICO/PEERS (aria2-next / libtorrent)
echo Log: "%LOG%"
echo Destino: "%DEST%"
echo.
echo Este teste NAO altera os testes de download direto nem o diagnostico v1 antigo.
echo Ele aumenta a busca por peers e reduz espera em trackers mortos.
echo Ele tambem prioriza o primeiro/ultimo 1%% de cada arquivo para permitir
echo pre-visualizacao no VLC durante o download (--bt-first-last-piece-first).
echo Pressione Ctrl+C depois de alguns minutos se quiser apenas medir velocidade.
echo.

"%BASE%aria2-next.exe" --version >> "%LOG%" 2>&1

"%BASE%aria2-next.exe" ^
  --no-conf=true ^
  --file-allocation=none ^
  --seed-time=0 ^
  --enable-dht=true ^
  --enable-peer-exchange=true ^
  --bt-enable-lpd=true ^
  --bt-max-peers=200 ^
  --bt-tracker-completion-timeout=10 ^
  --bt-tracker-receive-timeout=15 ^
  --bt-first-last-piece-first=true ^
  --state-dir="%DEST%\state" ^
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
