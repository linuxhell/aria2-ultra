# aria2-ultra

Fork experimental do aria2, com foco em:

- preservar o desempenho de download direto validado no pacote baseline do Windows;
- manter compatibilidade com BitTorrent v1;
- adicionar BitTorrent v2 e torrents híbridos;
- produzir builds x64 para Windows, Linux e macOS.

## Baseline imutável de referência

Pacote recebido em 2026-09-28: `aria2-ultra-diagnostico-v5-windows-x64 (1)(1).zip`.

- `aria2c.exe` SHA-256: `fdf2aa5a2f17c1746c1437210f54e5d7a18d77f5d41d7d52dc901a62e43b88dd`
- upstream indicado pelo próprio pacote: `aria2/aria2@9e7273583f83e881e3ec067b523ba88724088d2f`
- teste de regressão principal: `teste-direto-16-trunc-autosave.cmd`
- parâmetros essenciais desse teste: `--file-allocation=trunc --auto-save-interval=60 --split=16 --max-connection-per-server=16 --min-split-size=1M`

O binário baseline não é substituído nem tratado como fonte. O desenvolvimento parte do commit upstream fixado e cada mudança será validada contra os testes de desempenho e protocolo.

## Política de desenvolvimento

O trabalho de BitTorrent v2 acontece em branch de desenvolvimento. Builds de teste podem ser executados automaticamente, mas o merge final em `main` só acontece após validação no Windows e regressões em Linux/macOS.
