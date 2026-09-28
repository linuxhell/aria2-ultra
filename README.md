# aria2-ultra

Fork experimental do aria2, com foco em:

- preservar o desempenho de download direto validado no pacote baseline do Windows;
- manter compatibilidade com BitTorrent v1;
- adicionar BitTorrent v2 e torrents híbridos;
- produzir builds x64 para Windows, Linux e macOS.

## Baseline imutável de referência

Pacote recebido em 2026-09-28: `aria2-ultra-diagnostico-v5-windows-x64 (1)(1).zip`.

- `aria2c.exe` SHA-256: `29a91e6ae814e754a1464fca00d8204c331e583c002de75ad72b640e8793f5cf`
- upstream indicado pelo próprio pacote: `aria2/aria2@9e7273583f83e881e3ec067b523ba88724088d2f`
- teste de regressão principal: `tests/teste-direto-16-trunc-autosave.cmd` (é o que comprova que o download direto do aria2c está otimizado; **não deve ser alterado**)
- parâmetros essenciais desse teste: `--file-allocation=trunc --auto-save-interval=60 --split=16 --max-connection-per-server=16 --min-split-size=1M`

O binário baseline (`aria2c.exe` na raiz) não é substituído nem tratado como fonte. Ele fica congelado aqui só como referência de comparação de desempenho.

## Código-fonte

O diretório `upstream/` contém o código-fonte do aria2 vendorizado exatamente no commit fixado acima (`aria2/aria2@9e72735`, aria2 1.37.0), sem modificações. É a base de onde o desenvolvimento do aria2-ultra parte.

Todo o trabalho de BitTorrent v2/híbrido (BEP 52) e demais melhorias acontece em cima dessa árvore, preservando:

- o comportamento de download direto (mesmos parâmetros validados pelo teste de regressão);
- compatibilidade com BitTorrent v1 (magnet/`.torrent` clássicos continuam funcionando sem mudança de comportamento).

## Política de desenvolvimento

O trabalho de BitTorrent v2 acontece em branch de desenvolvimento. Builds de teste podem ser executados automaticamente, mas o merge final em `main` só acontece após validação no Windows e regressões em Linux/macOS.
