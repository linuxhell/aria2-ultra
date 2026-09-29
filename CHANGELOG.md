# Changelog

🌐 [English](CHANGELOG.en.md) | [简体中文](CHANGELOG.zh-CN.md)

Todas as mudanças notáveis do aria2-ultra em relação ao aria2 clássico são documentadas aqui.

## [Não lançado]

### Adicionado
- Suporte nativo a BitTorrent v2 e híbrido (BEP 52) sobre a stack de BitTorrent do aria2 clássico: parsing de metadados/magnet v2, verificação de integridade via Merkle root e piece layers, e infohash de 20 bytes correto nos caminhos de protocolo (handshake, tracker, DHT, peer-wire).
- Builds oficiais totalmente estáticos para Windows, Linux e macOS (x64), publicados via CI (`.github/workflows/bep52-phase4.yml`), sem nenhuma DLL/biblioteca dinâmica de terceiros ao lado do binário.

### Alterado
- `-a/--file-allocation` agora usa `trunc` como padrão para download direto (antes: `prealloc`), eliminando o atraso de alocação/zeragem de arquivo antes do primeiro byte.
- Downloads de BitTorrent (v1, v2 e híbrido) rebaixam automaticamente `--file-allocation` para `none`, já que peças são escritas fora de ordem em múltiplos arquivos — alocação antecipada não ajuda em nada nesse caso. Um valor explícito passado pelo usuário sempre prevalece.

### Removido
- Binário `aria2c.exe` de referência e `SHA256SUMS.txt` que estavam versionados na raiz do repositório — binários agora só são distribuídos como assets de Release, nunca commitados.
- Documentação e scripts de teste referentes à abordagem descartada baseada em aria2-next/libtorrent-rasterbar (o fork atual segue com a base do aria2 clássico).
- Notas internas de planejamento e handoff (`BT-V2-PLANO.md`, `HANDOFF-CHATGPT.md`) que não faziam sentido fora do processo de desenvolvimento.

## Base

Vendorizado a partir do aria2 clássico (`aria2/aria2`), com os patches acima aplicados por cima.
