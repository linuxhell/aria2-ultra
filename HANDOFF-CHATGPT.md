# Handoff para o ChatGPT — aria2-ultra (retomada 2026-09-29)

Este documento existe para dar contexto completo a um chat novo do ChatGPT sem histórico. Leia inteiro antes de mexer em qualquer coisa. Se este arquivo e o estado real do repositório (`git log`, PR aberto, CI) divergirem, **confie no repositório**, não neste texto.

Existe um documento irmão deste, `HANDOFF-CHATGPT.md`, no repositório `apocalipse-download-manager` (branch `prep-aria2-ultra`) — leia os dois, são complementares. Aqui é o fork do aria2; lá é o app desktop que consome os binários publicados por este repo.

## O que é o aria2-ultra

Fork do [aria2](https://github.com/aria2/aria2) clássico (base autotools/automake, C++11), **não** do aria2-next/libtorrent-rasterbar — essa abordagem foi tentada e abandonada cedo na história do projeto (ver `git log --oneline` dos commits antigos "Troca base vendorizada..." / "Volta base para aria2 classico"). Todo o trabalho atual é em cima do aria2 clássico vendorizado em `upstream/`.

Objetivo do fork: adicionar suporte nativo a **BitTorrent v2 e híbrido (BEP 52)** diretamente na stack de BitTorrent já existente do aria2 clássico, sem trocar de engine.

## Estrutura de branches

- **`claude/upbeat-cannon-7ovk29`** — branch principal/base do repositório GitHub (`linuxhell/aria2-ultra`). README/CHANGELOG em pt-BR/en/zh-CN, `.gitignore`, repo limpo de lixo (binário `aria2c.exe` de referência e afins já foram removidos do histórico daqui pra frente). README já tem: tabela de desempenho medido (clássico vs aria2-next vs aria2-ultra, ver seção 2 abaixo) e seção de doação (PayPal `jv12802@gmail.com`).
  ⚠️ **Essa branch está à frente de `chatgpt/bep52-phase4` só nesses arquivos de documentação** (README/CHANGELOG/.gitignore) — o trabalho de código (BEP52, defaults, CI estático) está em `chatgpt/bep52-phase4` e ainda não foi mergeado pra cá via PR #1. As duas branches vão precisar convergir em algum momento; não presuma que uma tem tudo que a outra tem.
- **`chatgpt/bep52-phase4`** — branch de desenvolvimento onde este trabalho está acontecendo, com PR #1 aberto contra `claude/upbeat-cannon-7ovk29`. **É aqui que você deve continuar.**
- O diretório de trabalho local usado nesta sessão é `/tmp/interop-build` (checkout de `chatgpt/bep52-phase4`).

## O que já está pronto e validado nesta branch

### 1. BEP52 / BitTorrent v2 (fases 1-4)
Parsing de metadados/magnet v2, verificação Merkle root/piece layers, infohash de 20 bytes correto em todos os caminhos de protocolo (handshake, tracker, DHT, peer-wire). Testado pelo usuário em hardware real: torrent v1/v2 funcionando, sem erros relacionados a hash.

### 2. Defaults de performance ajustados na fonte (não é mais preciso passar flag nenhuma)
Todos em `upstream/src/OptionHandlerFactory.cc`:

- `-a/--file-allocation`: default mudou de `prealloc` para **`trunc`**. `upstream/src/RequestGroup.cc::setDownloadContext()` faz downgrade automático pra `none` especificamente em downloads BitTorrent (peças fora de ordem em vários arquivos não se beneficiam de alocação antecipada), só quando o valor ainda é o default compilado — um `--file-allocation` explícito do usuário sempre prevalece.
- `-x/--max-connection-per-server`: `1` → **`16`**.
- `-s/--split`: `5` → **`16`**.
- `-k/--min-split-size`: `20M` → **`1M`** (senão qualquer arquivo abaixo de ~320M nunca alcançaria 16 segmentos, mesmo com `-s 16`).

Motivo: o usuário testou lado a lado (dois `.log` reais, 8.17GB ISO do Windows 11) — com 1 conexão (default antigo) apareceu uma queda real de throughput (~100MB/s → ~62MB/s) entre 80-88% do download, recuperando depois; com 16 conexões ficou uma linha praticamente reta em ~117.7MB/s do 0% ao 100%, 47% mais rápido no total (71s vs 103s). Confirmado via timestamp real de recebimento de rede no log (`WrDiskCacheEntry cache goff=`), não é artefato de log.

O usuário também rodou o mesmo arquivo no **aria2-next** (`--stream-max-connections=16 --file-allocation=trunc`, confirmado por print de tela com `CN:16`), pra comparação justa: 88,98s (~91,8MB/s) contra os 71s (~115,1MB/s) do aria2-ultra — **aria2-ultra ~20% mais rápido que o aria2-next**. Esses três números (clássico/aria2-next/aria2-ultra) já estão documentados como tabela no README (branch `claude/upbeat-cannon-7ovk29`, ver abaixo).

Esses três valores + o `file-allocation=trunc` batem exatamente com o baseline já validado em `tests/teste-direto-16-trunc-autosave.cmd` (que passa esses parâmetros explicitamente) — **esse script não deve ser alterado**, ele é a referência de regressão. Os scripts de torrent em `tests/teste-torrent*.cmd` usam `--file-allocation=none` explicitamente, consistente com o downgrade automático.

Verificação feita: rebuild completo + `make check` (todos passando) + `--help=#all` confirmando os novos defaults, em cada um dos commits que tocaram nesse arquivo.

### 3. Build 100% estático nas 3 plataformas (CI)
Arquivo: `.github/workflows/bep52-phase4.yml`, três jobs (`linux-tests`, `macos-x64`, `windows-x64`).

**Requisito não-negociável do usuário**: o binário final não pode ter nenhuma DLL/`.so`/`.dylib` solta ao lado — tudo estaticamente linkado, exceto o que cada plataforma exige por natureza (macOS sempre depende dinamicamente de `libSystem`/frameworks da Apple; isso é inevitável e aceito).

Mecanismo usado, depois de duas tentativas erradas:
- **Não** use `LDFLAGS="-static" --disable-shared --enable-static` manualmente — isso NÃO funciona neste `Makefile` gerado (o `LDFLAGS` passado ao `./configure` não chega na linha de link final do `aria2c`; descoberto inspecionando o `src/Makefile` gerado, que mostrava `LDFLAGS = ` vazio).
- **Use o mecanismo oficial do próprio aria2**: `./configure ARIA2_STATIC=yes` (documentado no `README.rst` do próprio aria2: "To build statically linked aria2, use ARIA2_STATIC=yes"). Isso ativa `pkg-config --static` E adiciona `-all-static` do libtool à linha de link — só isso força de verdade cada dependência de terceiros (OpenSSL, libxml2, sqlite3, c-ares, libssh2, gcrypt, gmp...) a linkar estaticamente.

Detalhes por plataforma:
- **Linux** (`ubuntu-24.04`): `./configure ARIA2_STATIC=yes`. Precisou instalar `liblzma-dev` (dependência transitiva estática do OpenSSL via `pkg-config --static`, não vem com as outras `-dev`) e criar um symlink `libcares.a -> libcares_static.a` (o `libc-ares-dev` do Ubuntu 24.04 é buildado via CMake e nomeia o `.a` de forma não-convencional; sem o symlink, `-lcares` não acha o arquivo). Verificação: `ldd` no binário final não pode reportar nenhuma lib fora de `libc/libm/libpthread/libdl/librt/ld-linux`.
- **Windows** (`windows-2022`, MSYS2 MINGW64 nativo): `LDFLAGS="-static-libgcc -static-libstdc++" ./configure --host=x86_64-w64-mingw32 --with-wintls --without-openssl --disable-websocket ARIA2_STATIC=yes`. Cuidado histórico: `src/aria2c.exe` na árvore de build é um stub do libtool (só funciona junto de `src/.libs/`), o binário de verdade só existe depois de `make install DESTDIR=...`. Verificação: `ldd` no `.exe` final só pode mostrar DLLs de `C:\Windows\`.
- **macOS** (`macos-13`): Apple/ld64 não tem modo `-static` de verdade (dependência dinâmica de `libSystem`/frameworks é obrigatória e aceita). Truque usado: antes de configurar, renomear/esconder os `.dylib` de cada fórmula Homebrew (`openssl@3 libxml2 sqlite c-ares libssh2 gmp libgcrypt`) pra forçar o linker a usar o `.a` estático que o Homebrew também instala. Depois `./configure ARIA2_STATIC=yes --disable-websocket` com `PKG_CONFIG_PATH` apontando pros `.pc` de cada fórmula. Verificação: `otool -L` no binário só pode mostrar dependências em `/usr/lib/` ou `/System/`.

**Status do CI nesta sessão** (run mais recente no momento deste handoff, run id `36605585967`, commit `ec6eba7`): Linux ✅ passou, Windows ✅ passou, **macOS ficou mais de 2 horas parado em `queued`** sem sequer começar a rodar — é fila normal de runner `macos-13` na GitHub (aconteceu em praticamente todo run desta sessão), não é bug do workflow, mas é bom saber que pode demorar bastante antes de assumir que algo travou. **Confira o status atual antes de assumir qualquer coisa**:
```
gh api repos/linuxhell/aria2-ultra/actions/runs?branch=chatgpt/bep52-phase4&per_page=1
# ou pela UI: https://github.com/linuxhell/aria2-ultra/actions/workflows/bep52-phase4.yml
```
⚠️ O workflow tem `concurrency: cancel-in-progress: true` por branch — qualquer push novo cancela o run anterior inteiro, incluindo um macOS que já estava rodando/na fila. Evite pushes desnecessários enquanto o macOS estiver rodando.

## O que falta fazer (nesta ordem)

1. **Confirmar CI verde nas 3 plataformas** no commit mais recente de `chatgpt/bep52-phase4` (atualmente `ec6eba7`, mas confira `git log -1` porque pode ter avançado).
2. **Publicar uma Release real no GitHub** (`linuxhell/aria2-ultra`) — o usuário já aprovou isso explicitamente numa sessão anterior. Nomeação dos assets tem que bater com o que o `apocalipse-download-manager` espera (ver `apps/desktop/src-tauri/src/main.rs`, função `aria2_asset_suffix()`, e `.github/workflows/release.yml`/`test-build.yml` desse outro repo):
   - `aria2c-windows-x64.exe` + `aria2c-windows-x64.exe.sha256`
   - `aria2c-linux-x64` + `aria2c-linux-x64.sha256`
   - `aria2c-macos-x64` + `aria2c-macos-x64.sha256`
   Os artifacts do CI atual saem nomeados de forma diferente (`aria2-ultra-bep52-{linux,macos,windows}-x64`, contendo `dist/aria2c` ou `dist/aria2c.exe`) — vai precisar renomear/reempacotar na hora de subir os assets da Release, ou ajustar o workflow pra já produzir com o nome final.
3. **Depois da Release publicada**, ir para o repositório `apocalipse-download-manager` e atualizar `aria2_release_repo` lá — ver a seção abaixo e o handoff próprio desse repo.

## `apocalipse-download-manager` — o app que consome este fork (repo irmão, branch `prep-aria2-ultra`)

Não é este repositório, mas depende diretamente dele: é o app desktop (Tauri, Rust + browser extension) que baixa o `aria2c` publicado aqui como Release asset e o usa como motor de download. Tem um `HANDOFF-CHATGPT.md` próprio lá com todo o detalhe; resumo do que já foi feito nessa mesma leva de trabalho:

- **Bug real corrigido**: o app forçava `--file-allocation=none` tanto no daemon (`apps/desktop/src-tauri/src/aria2.rs`, `spawn_once`) quanto em cada download HTTP/FTP (`add_download()`), anulando silenciosamente o novo default `trunc` deste fork — ou seja, mesmo com este fork mais rápido, o app nunca exercitava isso. Corrigido: os dois overrides foram removidos, o daemon agora herda o default do próprio aria2-ultra.
- **Logs de diagnóstico corrigidos**: antes diziam `backend=classic` e `fileAllocation=none` fixo mesmo quando não era mais verdade. Agora registram a versão real via `aria2.getVersion()`, `backend=aria2-ultra`, e o `fileAllocation` real por tipo de download (`trunc` pra direto, `none` pra BT). Também passou a logar a mensagem de erro do aria2 quando um download falha (`status=error/removed`), que antes só sobrevivia se estivesse por acaso no `aria2.log`.
- **`connections_per_download` no app já é 16 por padrão** (`main.rs`, `default_connections()`) — ou seja, quem baixa pela UI do app já usava 16 conexões via RPC. O problema de "só 1 conexão" que o usuário viu nos testes era só porque ele estava chamando o `aria2c.exe` **direto pelo `.cmd`**, sem passar flag nenhuma — daí a mudança de default #2 desta lista, feita aqui no aria2-ultra, também beneficiar qualquer uso direto do binário fora do app.
- **Interceptação do link de uso único do Rapidgator via Shift**: implementado com `declarativeNetRequest` (regra de rede temporária por aba, some sozinha) porque o link final é gerado via `location.href` pelo próprio JS do site, que é uma propriedade "unforgeable" no Chrome — não dá pra interceptar isso só com hooks de JavaScript de página (`fetch`/`click`/`window.open`, que já existiam e funcionam para outros casos). **Não testado contra o site real**, só revisão de código — precisa validação manual.
- **UI**: checkbox de "extrair ao terminar" só aparece se o download for detectado como arquivo compactado; controles de `.torrent` (salvar/limpar) em `data/torrents` (ou `data\torrents` no Windows), path já tratado corretamente via `PathBuf::join` do Rust.
- **`aria2_release_repo`** (setting em `main.rs`, default atualmente `"FerroDownload/aria2-static-builds"`) — **ainda não foi trocado** pra apontar pro `linuxhell/aria2-ultra`. Depende do passo 2 da lista acima (Release publicada primeiro). Não decidido ainda se muda só o default do setting, ou também o fetch hardcoded em `.github/workflows/release.yml`/`test-build.yml` desse repo — perguntar ao usuário se não estiver claro no handoff de lá.
- **Gap real identificado, sem código ainda**: os logs desse app não distinguem torrent v1/v2/híbrido (nenhum campo tipo `metaVersion` do RPC é capturado). Então mesmo com os logs corrigidos, se o BEP52 (a razão de existir deste fork) tiver um bug específico de v2/híbrido, não dá pra provar isso só pelos logs — precisaria de teste manual com um torrent v2/híbrido conhecido, ou de instrumentação nova ali.

## Regras que valem para qualquer trabalho futuro aqui

- **Nunca commitar binário compilado no repositório.** Binários só existem como artifact de CI ou asset de Release.
- **Sempre consultar os scripts de teste validados** (`tests/teste-direto-16-trunc-autosave.cmd` para download direto, `tests/teste-torrent*.cmd` para torrent) antes de mexer em qualquer parâmetro de performance — eles são a fonte da verdade do que já foi comprovado funcionar bem, não adivinhe.
- **Todo binário final tem que ser estático** nas 3 plataformas, sem exceção (além do que cada SO exige nativamente).
- Este arquivo deve ser mantido atualizado ou apagado quando ficar obsoleto — não deixe virar lixo desatualizado igual ao anterior (que descrevia uma arquitetura aria2-next que não existe mais neste repo e foi removido por isso).
