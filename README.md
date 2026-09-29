# aria2-ultra

🌐 [English](README.en.md) | [简体中文](README.zh-CN.md)

Fork do [aria2](https://aria2.github.io/) clássico com suporte nativo a **BitTorrent v2 e híbrido (BEP 52)**, builds totalmente estáticos para Windows, Linux e macOS (x64), e padrões de desempenho já ajustados e validados para download direto e torrent.

## Por que este fork existe

O aria2 clássico nunca implementou BitTorrent v2/híbrido (BEP 52). O aria2-ultra adiciona esse suporte diretamente em cima da stack de BitTorrent já existente no aria2 (sem trocar de engine, sem depender de bibliotecas externas como libtorrent-rasterbar), preservando 100% de compatibilidade com torrents v1 e com todas as demais funcionalidades do aria2.

## Binários

O diretório `upstream/` contém o código-fonte do aria2 clássico (autotools/automake), vendorizado a partir de um commit fixo, com os patches do aria2-ultra aplicados por cima.

Baixe os binários prontos na [página de Releases](../../releases). Cada plataforma é publicada seguindo suas próprias convenções nativas:

| Plataforma | Arquivo | Observação |
| --- | --- | --- |
| Windows x64 | `aria2c-windows-x64.exe` | Um único `.exe`, sem nenhuma DLL solta ao lado — tudo (libaria2, OpenSSL/wintls, libxml2, sqlite3, c-ares, libssh2, zlib, runtime do compilador) linkado estaticamente dentro do binário. |
| Linux x64 | `aria2c-linux-x64` | Binário estático (sem dependências dinâmicas além das bibliotecas base do sistema). |
| macOS x64 | `aria2c-macos-x64` | Todas as dependências de terceiros (OpenSSL, libxml2, sqlite3, c-ares, libssh2, gmp, gcrypt) linkadas estaticamente. Só permanece dinâmico o vínculo com as bibliotecas do próprio sistema Apple (`libSystem`/frameworks), que a Apple exige e nenhum app pode evitar — cada plataforma respeita as convenções do seu próprio formato de binário.

Cada asset vem acompanhado de um `.sha256` para verificação de integridade. As três plataformas são construídas e verificadas automaticamente pelo workflow [`bep52-phase4.yml`](.github/workflows/bep52-phase4.yml).

## Parâmetros padrão otimizados

- **Download direto**: `-a/--file-allocation` usa `trunc` por padrão (reserva o espaço em disco quase instantaneamente, sem o atraso do `prealloc`/`falloc` antes do primeiro byte). Baseline de regressão validada: [`tests/teste-direto-16-trunc-autosave.cmd`](tests/teste-direto-16-trunc-autosave.cmd) — **não deve ser alterado**, é a referência de desempenho.
- **BitTorrent (v1, v2 e híbrido)**: o mesmo `--file-allocation` é automaticamente rebaixado para `none` quando o download é um torrent, já que peças são escritas fora de ordem em vários arquivos e alocação antecipada só atrasa o início. Um `--file-allocation` explícito passado pelo usuário sempre prevalece sobre esse ajuste automático.
- Scripts de referência e diagnóstico de torrent (v1 e v1/v2 misto) estão em [`tests/`](tests/).

## Desenvolvimento

Melhorias acontecem em branch de desenvolvimento; o merge final na branch principal só ocorre após validação em Windows e regressões em Linux/macOS via CI.

## Créditos

Baseado no [aria2](https://github.com/aria2/aria2) original, de Tatsuhiro Tsujikawa e colaboradores.
