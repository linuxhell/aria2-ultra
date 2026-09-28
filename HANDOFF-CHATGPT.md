# aria2-ultra — arquivo de retomada para o ChatGPT

Documento gerado pelo Claude (Anthropic) para dar continuidade ao trabalho no
aria2-ultra. Contém tudo que foi feito, tudo que foi testado e a próxima
tarefa concreta: aplicar uma correção no código-fonte do aria2-next e
**compilar um `aria2-next.exe` para Windows x64** (algo que o Claude não
conseguiu fazer porque o sandbox onde ele rodou só tem toolchain Linux).

Repositório: https://github.com/linuxhell/aria2-ultra
Branch: `claude/upbeat-cannon-7ovk29`
Último commit no momento deste documento: `12f3b6e`

---

## 1. Contexto do projeto

O usuário (dono do repo `linuxhell/aria2-ultra`) já tinha um pacote baseline
Windows x64 do **aria2 clássico** (`aria2c.exe`, compilado a partir de
`aria2/aria2@9e72735`, aria2 1.37.0), otimizado previamente pelo próprio
ChatGPT para download direto (HTTP). Esse binário/config é o "padrão-ouro"
de velocidade de download direto e **não deve ser alterado**.

Pedido original do usuário:
1. Testar e corrigir o script de diagnóstico de torrent v1
   (`teste-torrent-v1-diagnostico.cmd`).
2. Depois de v1 validado, implementar suporte a **BitTorrent v2 / híbrido
   (BEP 52)**, sem mexer no download direto nem no v1 clássico.
3. Um bug relatado: com torrent v1, ao chegar a ~100 MB baixados no disco, o
   VLC não conseguia abrir a pré-visualização do vídeo.
4. Outro bug relatado: o log do diagnóstico de torrent v1 passava de 300 MB.

## 2. O que já foi feito (commits `bbc2345` → `12f3b6e`)

### 2.1. Correções no aria2c clássico (v1)
Arquivo `teste-torrent-v1-diagnostico.cmd`:
- `--log-level=debug` → `--log-level=info` (o `debug` gravava todo o
  tráfego de protocolo BitTorrent, causando o log de 300+ MB).
- Adicionado `--bt-prioritize-piece=head=10M,tail=1M` (sem isso, o aria2
  baixa peças em ordem "rarest-first"; mesmo com 100 MB no disco, o início
  do arquivo de vídeo podia estar incompleto, por isso o VLC não abria).

Esse script já foi testado pelo usuário e funcionou.

### 2.2. Decisão de arquitetura: aria2-next em vez de reimplementar BEP 52

O aria2 clássico **nunca implementou BitTorrent v2/híbrido** — seria
necessário escrever esse suporte do zero em cima do stack BT próprio do
aria2 (meses de trabalho). Em vez disso, adotamos como base o fork mantido
**aria2-next** (https://github.com/AnInsomniacy/aria2-next), que já
substituiu toda a stack de BitTorrent por **libtorrent-rasterbar 2.1**,
com suporte nativo a v1, v2 e híbrido (detecta o formato sozinho pelo
info-hash/metainfo, sem flag especial).

- `upstream/` no repo aria2-ultra agora contém o código-fonte do aria2-next
  vendorizado (vendored) no commit `f58a2d9463b3b549ca19039b055df1448e1b8c46`
  / tag `v2.8.3` (sem modificações locais ainda — ver seção 4 para a
  modificação proposta).
- O aria2 clássico (autotools) que estava vendorizado antes foi removido.
- **Atenção a secret scanning do GitHub**: ao vendorizar o aria2-next,
  o push original foi bloqueado pelo GitHub Push Protection por causa de
  chaves privadas de demonstração antigas do OpenSSL/nghttp2 dentro de
  `third_party/` (fixtures de teste, não segredos reais, mas o scanner
  bloqueia mesmo assim). Foram removidos antes do commit:
  - `upstream/third_party/nghttp2/integration-tests/{server.key,alt-server.key}`
  - `upstream/third_party/openssl/apps/{client.pem,ca-key.pem,dsa-ca.pem,s512-key.pem,s1024key.pem,dsa-pca.pem,privkey.pem,rsa8192.pem,pca-key.pem,server2.pem,server.pem}`
  Se for reclonar o aria2-next puro de novo, remova esses arquivos (ou
  arquivos equivalentes) antes de tentar dar `git push`.

### 2.3. Build do aria2-next testado no Linux (só validação, não é o alvo final)

Não havia binário Windows pronto à mão durante os testes do Claude, então
ele baixou o binário oficial Linux x86_64 da release v2.8.3
(`https://github.com/AnInsomniacy/aria2-next/releases/download/v2.8.3/aria2-next-2.8.3-linux-x86_64`)
e testou comandos reais com ele, só para validar que as flags traduzidas
funcionam (não precisa refazer isso, foi só uma etapa de validação).

### 2.4. Tradução de flags: aria2 clássico → aria2-next

O aria2-next tem um "Legacy Input Adapter"
(`upstream/src/LegacyInputAdapter.cc`) que traduz automaticamente a maioria
das flags antigas do aria2 para os nomes novos, emitindo um aviso no log.
Mapeamento relevante para o teste de download direto:

| Flag clássica | Flag nativa aria2-next | Observação |
| --- | --- | --- |
| `--split` + `--max-connection-per-server` | `--stream-max-connections` | usa o menor dos dois valores, capado em 256 |
| `--auto-save-interval` | `--state-save-interval` | |
| `--min-split-size` | *(nenhuma)* | **aposentada**, ignorada silenciosamente — o motor decide o tamanho de faixa HTTP sozinho (`--stream-max-range-size=0` = automático) |
| `--file-allocation`, `--continue`, `--auto-file-renaming`, `--no-conf` | mesmos nomes | sem mudança |
| `--bt-prioritize-piece=head,tail` | `--bt-first-last-piece-first=true` | booleano fixo (1% do arquivo), não aceita tamanho customizado como a flag antiga |
| `--bt-request-peer-speed-limit`, `--bt-save-metadata`, `--bt-timeout`, etc. | *(nenhuma)* | aposentadas, veja a lista `RETIRED` em `LegacyInputAdapter.cc` linha ~499 |

### 2.5. Scripts novos criados (não alteram os antigos)

- `tests/teste-direto-16-trunc-autosave-next.cmd`: mesma regressão de
  download direto (16 conexões, trunc, auto-save 60s), usando os nomes
  nativos do aria2-next e `--state-dir` isolado por execução. **Corrigido**
  para funcionar tanto solto na mesma pasta do `aria2-next.exe` quanto
  dentro de uma subpasta `tests\` (bug relatado pelo usuário: o script
  original só procurava o `.exe` uma pasta acima, e se não achasse, saía
  sem `pause`, parecendo que "não abria").
- `teste-torrent-v1v2-diagnostico-next.cmd`: diagnóstico de peers/trackers
  equivalente ao v1 clássico, mas rodando sobre libtorrent — funciona com
  torrent v1, v2 e híbrido sem distinção. Usa
  `--bt-first-last-piece-first=true` para preview no VLC. **Testado pelo
  usuário e funcionou perfeitamente** (torrent de 1,9 GiB completo em ~1 min).

O binário `aria2-next.exe`/`aria2-next` não está no repositório — precisa
ser baixado (release oficial) ou compilado à parte e colocado do lado dos
scripts.

## 3. O problema em aberto: download direto no aria2-next é ~9% mais lento

O usuário rodou os dois testes de download direto (mesma ISO do Windows 11,
back-to-back, mesmas condições de rede) e comparou os logs:

| | Início | Fim | Duração |
| --- | --- | --- | --- |
| `teste-direto-16-trunc-autosave.cmd` (aria2c clássico) | 16:11:01,42 | 16:12:15,70 | **74,3 s** |
| `teste-direto-16-trunc-autosave-next.cmd` (aria2-next) | 16:12:26,53 | 16:13:47,75 | **81,2 s** |

Sintoma visual relatado: no aria2-next, o download "começou devagar e foi
acelerando", diferente do aria2c clássico, que já é rápido do início ao fim.

### 3.1. Investigação feita (código-fonte, `upstream/src/`)

1. **Não é um "slow start" deliberado por design.** Em
   `upstream/src/CurlDownloadImpl.h` e `upstream/src/stream/StreamStorage.cc`
   linha ~218, `impl.connectionLimit` é inicializado **igual a**
   `impl.maxConnections` (ou seja, começa já no máximo pedido, não sobe
   gradualmente de 1). O mecanismo de "reward/penalize connection limit" em
   `upstream/src/stream/StreamScheduling.cc` só REDUZ o limite reativamente
   se o servidor responder HTTP 429/503, e depois recupera 1 conexão por
   segundo — mas isso só entra em ação se houver overload real do servidor.

2. **Confirmado por teste real**: o Claude baixou um arquivo grande (~1 GB,
   tarball do LLVM, via GitHub Releases, com as mesmas flags traduzidas:
   `--stream-max-connections=16 --state-dir=... --file-allocation=trunc
   --state-save-interval=60 --continue=false --auto-file-renaming=false`)
   rodando o binário Linux do aria2-next. Resultado: `CN:16` (16 conexões
   simultâneas) já no primeiro segundo de execução, sem nenhuma rampa,
   velocidade média de 162 MiB/s. **No Linux não há slow start algum.**

3. **Causa raiz mais provável, achada em
   `upstream/src/transport/CurlOptions.cc` linhas 17-23**:

   ```cpp
   long platformSslOptions() noexcept
   {
   #ifdef _WIN32
     return CURLSSLOPT_REVOKE_BEST_EFFORT;
   #else
     return 0L;
   #endif
   }
   ```

   Essa flag é aplicada em **toda conexão TLS**, no Windows, via
   `CURLOPT_SSL_OPTIONS`/`CURLOPT_PROXY_SSL_OPTIONS` (mesma função,
   chamada em `configureTls()`, linhas ~55-70 do mesmo arquivo). No
   Windows, o libcurl usa Schannel (confirmado no log do usuário:
   `libcurl/8.21.0(Schannel;threaded DNS)`), e com
   `CURLSSLOPT_REVOKE_BEST_EFFORT`, o Schannel **tenta** checar revogação
   de certificado (OCSP/CRL) em cada handshake TLS — só não falha a conexão
   se o serviço de revogação estiver inacessível, mas ainda assim **espera**
   a tentativa (ou o timeout dela) antes de completar o handshake. Isso é
   um problema de latência conhecido de libcurl+Schannel no Windows, e bate
   exatamente com a diferença observada: no Linux (onde essa flag nem
   existe — `0L`) não há atraso nenhum; no Windows, a primeira conexão
   (que trava o início do paralelismo — veja `configurePlanner()` em
   `StreamScheduling.cc`, que só libera as outras 15 conexões depois que a
   primeira confirma suporte a `Range` e o tamanho total do arquivo) fica
   sujeita a essa checagem de revogação em série, e as 16 conexões também
   pagam esse custo individualmente ao abrir.

   A correção padrão da comunidade curl para esse cenário é trocar
   `CURLSSLOPT_REVOKE_BEST_EFFORT` por `CURLSSLOPT_NO_REVOKE` (pula a
   checagem de revogação inteiramente, sem esperar por ela). Isso é uma
   troca segurança-por-velocidade real (deixa de detectar certificados
   revogados), então a recomendação é implementar como **flag opt-in**, não
   mudar o padrão silenciosamente.

### 3.2. Correção proposta (ainda NÃO aplicada no repositório)

Em `upstream/src/transport/CurlOptions.cc`, algo no espírito de:

```cpp
long platformSslOptions(const Option* option) noexcept
{
#ifdef _WIN32
  if (option->getAsBool(PREF_TLS_SKIP_REVOCATION_CHECK)) {
    return CURLSSLOPT_NO_REVOKE;
  }
  return CURLSSLOPT_REVOKE_BEST_EFFORT;
#else
  (void)option;
  return 0L;
#endif
}
```

e então:
1. Adicionar uma nova preferência `PREF_TLS_SKIP_REVOCATION_CHECK` /
   `--tls-skip-revocation-check=[true|false]` (default `false`, para não
   mudar comportamento/segurança por padrão) em `prefs.h`/`prefs.cc` e no
   parser de opções (ver como outras opções booleanas de TLS, tipo
   `--check-certificate`, estão registradas em `src/options/` e
   `src/usage_text.h`, e documentar em
   `docs/manual/en/aria2-next.rst` ao lado de `--check-certificate`).
2. Atualizar a chamada `set(CURLOPT_SSL_OPTIONS, platformSslOptions());` e
   `set(CURLOPT_PROXY_SSL_OPTIONS, platformSslOptions());` em
   `configureTls()` para passar a `Option*` já disponível na função.
3. Seguir as convenções do projeto descritas em `upstream/AGENTS.md`
   (C++17, CMake é o único build system suportado, não adicionar
   Autotools, manter comentários em inglês explicando o porquê, rodar
   `cmake --preset default && cmake --build --preset default && ctest
   --preset default` antes de considerar pronto).

**Isso ainda não foi implementado nem testado** — é a próxima tarefa.

## 4. Por que isso precisa ser retomado por outra ferramenta

O Claude rodou num sandbox **Linux only**, sem toolchain Windows. O
aria2-next é compilado nativamente no Windows via MSYS2 + clang
(`Compiler: clang 22.1.5 ... built by x86_64-Windows`, conforme o próprio
log de versão). Não é uma questão de cross-compile trivial — o pipeline
oficial deles (`docs/CONTRIBUTING.md`, `README.md` do aria2-next) espera
MSYS2 fornecendo shell/Make/Perl e o toolchain nativo do Windows para
compilar libcurl, libtorrent, GPAC, FFmpeg etc. (superbuild que compila
tudo de `third_party/` do zero).

## 5. Tarefas concretas para o ChatGPT

1. **Ambiente**: configurar MSYS2 (ou equivalente) num Windows real com
   CMake 3.25+, Ninja, clang, Make, Perl — conforme
   `upstream/README.md` (seção "Build") e `upstream/docs/CONTRIBUTING.md`
   do próprio aria2-next.
2. **Clonar/atualizar** o código já vendorizado: pode partir do que já está
   em `upstream/` dentro de
   `https://github.com/linuxhell/aria2-ultra` (branch
   `claude/upbeat-cannon-7ovk29`), que é uma cópia exata do aria2-next
   v2.8.3 (`f58a2d9`), sem modificações.
3. **Aplicar a correção da seção 3.2** (flag `--tls-skip-revocation-check`
   ou nome equivalente) em `upstream/src/transport/CurlOptions.cc` e nos
   arquivos de opções relacionados.
4. **Compilar**:
   ```powershell
   cmake --preset default
   cmake --build --preset default
   ctest --preset default
   build/default/aria2-next --version
   ```
5. **Testar** rodando `tests/teste-direto-16-trunc-autosave-next.cmd` (já
   está no repositório) duas vezes com a mesma ISO da Microsoft: uma vez
   sem a flag nova (comportamento atual) e outra com
   `--tls-skip-revocation-check=true` adicionado ao comando dentro do
   script, comparando os tempos totais (`ULTRA TEST: begin=... end=...` no
   log) contra os **74,3 s** do aria2c clássico já medidos.
6. **Se a hipótese se confirmar** (tempo cai para perto de 74s com a flag
   nova): comitar a mudança em `upstream/`, atualizar
   `tests/teste-direto-16-trunc-autosave-next.cmd` para incluir a flag por
   padrão (ou documentar como recomendação no README), e enviar
   (`git push`) para a branch `claude/upbeat-cannon-7ovk29` do repositório
   `linuxhell/aria2-ultra`. **Cuidado com o Push Protection do GitHub**
   (seção 2.2) se precisar revendorizar third_party do zero.
7. **Se não confirmar**: documentar o resultado e considerar a alternativa
   já validada — usar `aria2c.exe` clássico para download direto e
   `aria2-next.exe` só para torrent v1/v2/híbrido (estratégia de "dois
   binários", cada um no que é melhor).

## 6. Arquivos-chave para consulta rápida

- `upstream/AGENTS.md` — regras de contribuição do aria2-next (build,
  versionamento, release, convenções de código).
- `upstream/src/transport/CurlOptions.cc` — configuração de TLS (o alvo da
  correção).
- `upstream/src/stream/StreamScheduling.cc` — escalonador de conexões HTTP
  (`configurePlanner`, `rewardConnectionLimit`, `penalizeConnectionLimit`).
- `upstream/src/LegacyInputAdapter.cc` — tradução de flags clássicas do
  aria2 para os nomes nativos do aria2-next.
- `upstream/docs/manual/en/aria2-next.rst` — manual de opções de linha de
  comando (procurar por `--check-certificate`, `--stream-max-connections`
  para ver o padrão de documentação a seguir).
- `README.md` (raiz do aria2-ultra) — visão geral do projeto e tabela dos
  scripts de teste.
- `teste-torrent-v1-diagnostico.cmd`, `teste-torrent-v1v2-diagnostico-next.cmd`,
  `tests/teste-direto-16-trunc-autosave.cmd`,
  `tests/teste-direto-16-trunc-autosave-next.cmd` — scripts de teste
  citados acima.
