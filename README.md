# aria2-ultra

Fork experimental do aria2 (via [aria2-next](https://github.com/AnInsomniacy/aria2-next)), com foco em:

- preservar o desempenho de download direto validado no pacote baseline do Windows;
- manter compatibilidade com BitTorrent v1;
- BitTorrent v2 e torrents híbridos (nativos via libtorrent-rasterbar no aria2-next);
- produzir builds x64 para Windows, Linux e macOS.

## Baseline imutável de referência

Pacote recebido em 2026-09-28: `aria2-ultra-diagnostico-v5-windows-x64 (1)(1).zip`.

- `aria2c.exe` SHA-256: `29a91e6ae814e754a1464fca00d8204c331e583c002de75ad72b640e8793f5cf`
- upstream indicado pelo próprio pacote: `aria2/aria2@9e7273583f83e881e3ec067b523ba88724088d2f`
- teste de regressão principal: `tests/teste-direto-16-trunc-autosave.cmd` (é o que comprova que o download direto do aria2c está otimizado; **não deve ser alterado**)
- parâmetros essenciais desse teste: `--file-allocation=trunc --auto-save-interval=60 --split=16 --max-connection-per-server=16 --min-split-size=1M`

O binário baseline (`aria2c.exe` na raiz) não é substituído nem tratado como fonte. Ele fica congelado aqui só como referência de comparação de desempenho.

## Código-fonte: aria2-next como base

O diretório `upstream/` contém o código-fonte do [aria2-next](https://github.com/AnInsomniacy/aria2-next) (fork mantido do aria2, build CMake/Ninja), vendorizado sem modificações. É a base de onde o desenvolvimento do aria2-ultra parte a partir de agora.

Motivo da troca: o aria2 clássico (vendorizado antes neste repositório, agora removido) nunca implementou BitTorrent v2/híbrido (BEP 52) — seria necessário escrever esse suporte do zero em cima do stack BT próprio do aria2. O aria2-next já resolve isso: ele substituiu a stack de BitTorrent inteira por **libtorrent-rasterbar 2.1**, que suporta v1, v2 e híbrido nativamente (basta apontar o mesmo magnet/`.torrent`, sem flag especial — libtorrent detecta o formato sozinho). HTTP/HTTPS passam a usar libcurl.

Isso muda os nomes de várias flags de linha de comando em relação ao aria2 clássico. O aria2-next tem um adaptador de compatibilidade que traduz automaticamente a maioria das flags antigas (`--split`, `--max-connection-per-server`, `--auto-save-interval`, etc.) para os nomes nativos, emitindo um aviso no log — exceto `--min-split-size`, que foi aposentada (o motor decide o tamanho de faixa HTTP automaticamente) e é apenas ignorada, sem erro.

## Scripts de teste

| Script | Binário | O que valida |
| --- | --- | --- |
| `tests/teste-direto-16-trunc-autosave.cmd` | `aria2c.exe` (baseline clássico) | Regressão de download direto — **não alterar** |
| `tests/teste-direto-16-trunc-autosave-next.cmd` | `aria2-next.exe` | Mesma regressão, com as flags traduzidas para os nomes nativos do aria2-next (`--stream-max-connections`, `--state-save-interval`, `--state-dir` isolado por execução) |
| `teste-torrent-v1-diagnostico.cmd` | `aria2c.exe` (baseline clássico) | Diagnóstico de peers/trackers em torrent v1 |
| `teste-torrent-v1v2-diagnostico-next.cmd` | `aria2-next.exe` | Mesmo diagnóstico, mas via libtorrent — funciona com torrent v1, v2 e híbrido sem distinção, e prioriza o início/fim de cada arquivo (`--bt-first-last-piece-first=true`) para permitir pré-visualização no VLC durante o download |
| `teste-torrent.cmd` | `aria2c.exe` (baseline clássico) | Teste manual genérico de torrent |

Os binários `aria2-next.exe`/`aria2-next` (Windows/Linux/macOS) não estão neste repositório — baixe a release correspondente em https://github.com/AnInsomniacy/aria2-next/releases e coloque na raiz do pacote de teste, ao lado do `aria2c.exe` baseline.

O binário baseline `aria2c.exe` continua servindo só como referência de comparação de desempenho e não deve ser substituído.

## Política de desenvolvimento

Melhorias em cima do aria2-next (parâmetros de download direto, diagnóstico e streaming de torrent, etc.) acontecem em branch de desenvolvimento. Builds de teste podem ser executados automaticamente, mas o merge final em `main` só acontece após validação no Windows e regressões em Linux/macOS.
