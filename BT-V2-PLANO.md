# Plano de implementação: BitTorrent v2 (BEP 52) do zero no aria2 clássico

> **Este é um arquivo de retomada para o ChatGPT continuar o trabalho.**
> Foi escrito pelo Claude (Anthropic) sem acesso a nenhuma conversa
> anterior sua com o usuário — é autocontido de propósito. O usuário quer
> que a implementação de código da Fase 1 em diante seja feita por você a
> partir daqui. Não é preciso pedir mais contexto ao usuário antes de
> começar: tudo que foi levantado está neste documento e em
> `HANDOFF-CHATGPT.md` (histórico da sessão anterior, sobre uma tentativa
> diferente — usar o aria2-next — que foi abandonada; leia a seção logo
> abaixo pra entender por quê, mas o trabalho a partir daqui é neste
> arquivo, na base do aria2 clássico).
>
> Ambiente: diferente do aria2-next (que só compila com toolchain nativo
> do Windows), **este código (aria2 clássico, autotools) compila e roda
> testes em Linux normalmente** — `autoreconf -i && ./configure && make
> check`. Se faltar `autopoint`, `apt-get install gettext` resolve (já
> validado numa sessão anterior). Use isso pra validar cada fase de
> verdade antes de comitar, não só ler o código.

Documento de retomada. Base: aria2 clássico vendorizado em `upstream/`
(commit `aria2/aria2@9e7273583f83e881e3ec067b523ba88724088d2f`, versão
1.37.0, build autotools), o mesmo que já tem a otimização de download
direto validada (`tests/teste-direto-16-trunc-autosave.cmd`) e BitTorrent
v1 nativo e funcional. **Não mexer em nenhum dos dois.**

Decisão tomada: abandonar o aria2-next como base (ele tinha v1/v2 prontos
via libtorrent, mas o download direto ficava ~9% mais lento no Windows por
causa de checagem de revogação de certificado via Schannel — ver
`HANDOFF-CHATGPT.md` para o histórico completo dessa investigação). Em vez
disso, o v2 será implementado como código novo, direto no aria2 clássico,
mantendo total compatibilidade com o v1 existente desde o início.

Repositório: https://github.com/linuxhell/aria2-ultra
Branch: `claude/upbeat-cannon-7ovk29`

## Recomendações diretas (leia antes de começar a codar)

Decisões já tomadas e testadas nesta sessão — não precisa reabrir essas
discussões, só seguir:

1. **Base é o aria2 clássico (`upstream/` como está agora), não o
   aria2-next.** O usuário já mandou abandonar a linha aria2-next de vez.
   Se você (ChatGPT) tinha commits nessa branch sobre o fix de Schannel no
   aria2-next (`a06138b`, `8ccaa8a`, `f770aba`) — eles foram
   intencionalmente superados por um merge (`139be26`, estratégia
   "ours"), o histórico continua no git mas **a árvore de arquivos atual
   é o aria2 clássico**. Não tente reintroduzir o aria2-next nem misturar
   as duas árvores.
2. **Nunca altere** `aria2c.exe` (o binário baseline), o script
   `tests/teste-direto-16-trunc-autosave.cmd`, nem os parâmetros de
   download direto já validados (`--file-allocation=trunc
   --auto-save-interval=60 --split=16 --max-connection-per-server=16
   --min-split-size=1M`). É o "padrão-ouro" de velocidade e já está
   correto.
3. **Não reescreva o parsing v1 existente.** Adicione os campos e funções
   v2 ao lado do que já existe em `TorrentAttribute`/`bittorrent_helper.cc`
   (extensão, não substituição). Um torrent v1 puro tem que continuar
   passando pelo mesmo caminho de código de sempre, byte a byte igual.
4. **Reaproveite infraestrutura existente em vez de reimplementar:**
   - Hash SHA-256: já existe via `MessageDigest::create("sha-256")`. Não
     adicione outra lib de hash.
   - Info-hash v2 = mesmo `bencode2::encode(infoDict)` já usado pro v1,
     só trocando o algoritmo de hash. Não escreva um bencode-encoder novo.
   - `FileEntry`/`DownloadContext` já existentes: mapeie o `file tree` do
     v2 para eles em vez de criar uma estrutura de arquivo paralela.
5. **Siga a BEP 52 ao pé da letra, com vetores de teste reais.** Não
   invente formato de árvore de merkle, padding ou magnet v2 por conta
   própria — implementações que "quase" seguem a spec quebram
   interoperabilidade com clientes reais (qBittorrent, libtorrent,
   Transmission). Use torrents de teste v2/híbridos reais e gerados por
   ferramentas de referência para validar, não só torrents sintéticos
   escritos à mão.
6. **Valide cada fase compilando e rodando `make check` no Linux antes de
   comitar** (autotools funciona aqui, diferente do aria2-next). Não
   avance de fase com testes quebrando.
7. **Ordem das fases importa**: não pule pra protocolo peer-wire (Fase 4)
   antes de ter parsing + hash v2 (Fase 1) e verificação de integridade
   merkle (Fase 2) sólidos e testados — o resto depende disso estar
   correto.
8. **Torrent híbrido é o caso mais delicado**: ele precisa responder
   corretamente tanto a peers que só falam v1 quanto a peers que só falam
   v2, ao mesmo tempo, com o mesmo conteúdo de arquivo. Teste sempre os
   três casos (v1-only, v2-only, híbrido), não só v2-only.
9. **Só depois da Fase 6 (validação end-to-end)** compile um `aria2c.exe`
   novo pra Windows e atualize os scripts `.cmd` de teste. Não gere
   binário novo no meio do caminho com fases incompletas.

---

## Por que isso é grande

BEP 52 (BitTorrent v2) muda a estrutura interna do protocolo: hash de
peça vira SHA-256 (em vez de SHA-1), a lista de peças vira uma **árvore de
merkle** por arquivo, a lista de arquivos vira uma **árvore de diretórios**
bencoded (`file tree`) em vez de uma lista plana, o info-hash vira 32 bytes
(SHA-256) em vez de 20 (SHA-1), existe um novo formato de magnet
(`btmh:` em vez de `btih:`), e o protocolo teve extensões novas de troca
de peça entre peers (pedido de camadas de hash). Torrents **híbridos**
precisam carregar as duas estruturas ao mesmo tempo e responder a peers v1
e v2 ao mesmo tempo, sem se contradizerem.

Isso é trabalho de várias semanas em ritmo normal de engenharia. Este
documento quebra em fases que compilam e testam de forma independente, pra
dar pra retomar em qualquer ponto sem perder contexto.

## Onde cada coisa vive hoje (v1), pontos de extensão pro v2

Levantamento já feito no código, pra não precisar redescobrir:

- **`upstream/src/TorrentAttribute.h`** — struct com os metadados do
  torrent (`infoHash`, `name`, `announceList`, `metadata` etc.). É aqui
  que entram os campos novos do v2 (ver Fase 1).
- **`upstream/src/bittorrent_helper.h` / `.cc`** — parsing do `.torrent`
  (bencode) e do magnet URI. Função central: `processRootDictionary()`
  em `bittorrent_helper.cc` linha ~413. É onde o `info` dict é lido,
  o info-hash é calculado e os `FileEntry` são extraídos
  (`extractFileEntries()`).
  - **Achado importante**: o info-hash v1 é calculado assim (linha
    ~432-438):
    ```cpp
    std::string encodedInfoDict = bencode2::encode(infoDict);
    unsigned char infoHash[INFO_HASH_LENGTH];
    message_digest::digest(infoHash, INFO_HASH_LENGTH,
                           MessageDigest::sha1().get(), encodedInfoDict.data(),
                           encodedInfoDict.size());
    ```
    Ou seja, ele re-serializa o dict bencode parseado (`bencode2::encode`)
    e faz hash disso. **O info-hash v2 é literalmente o mesmo processo,
    trocando `MessageDigest::sha1()` por `MessageDigest::create("sha-256")`
    sobre os MESMOS bytes** — BEP 52 define infohash v2 = SHA-256(bencode
    do mesmo dict `info`, agora com os campos v2 dentro). SHA-256 já existe
    como algoritmo suportado em `MessageDigest` (usado hoje para checksums
    de Metalink) — não precisa implementar hash, só usar.
  - `parseMagnet()` (linha ~900-960) hoje só entende
    `xt=urn:btih:<hash-v1>`. O v2 usa `xt=urn:btmh:1220<hash-v2-hex>`
    (prefixo multihash `0x12 0x20` = "sha-256, 32 bytes").
- **`upstream/src/MessageDigest.h`** — `MessageDigest::create("sha-256")`
  já existe e funciona (fábrica genérica de hash, já usada por checksums).
  Nenhuma dependência nova.
- **`upstream/src/PieceStorage.h`, `DefaultPieceStorage.*`, `Piece.cc`,
  `PieceHashCheckIntegrityEntry.*`** — onde a integridade da peça é
  verificada hoje (fixado em SHA-1 por peça, formato v1). É aqui que entra
  a verificação por árvore de merkle do v2 (Fase 3).
- **`upstream/src/Bt*Message.*`** (mais de 30 arquivos) — mensagens do
  protocolo peer-wire (handshake, bitfield, piece, extended messages etc.).
  É aqui que entram as mensagens novas do BEP 52 (`hash request`, `hashes`,
  `hash reject`) na Fase 4.
- **`upstream/test/BittorrentHelperTest.cc`** — suite de teste existente
  (CppUnit) pro parsing v1. Os testes do v2 devem seguir o mesmo padrão,
  no mesmo arquivo ou em `Bt2HelperTest.cc` novo.
- **Build**: autotools (`./configure && make`). Já confirmado que dá pra
  rodar `autoreconf -i` neste sandbox Linux (precisou `apt-get install
  gettext` pro `autopoint`; já resolvido uma vez, documentar se precisar
  de novo). **Diferente do aria2-next, este código compila e testa no
  Linux mesmo — dá pra validar de verdade a cada fase, sem depender de
  Windows.**

## Fases

### Fase 0 — Base (concluída nesta sessão)
- [x] Restaurar `upstream/` para o aria2 clássico (`9e72735`), removendo o
  vendoring do aria2-next.
- [x] Confirmar que o binário baseline (`aria2c.exe`), os testes de
  download direto e o diagnóstico v1 continuam intocados.

### Fase 1 — Parsing e hashes v2 (estruturas de dados, sem tocar no motor de download)
Objetivo: aria2-ultra consegue **ler** um `.torrent` v2 ou híbrido e um
magnet v2/híbrido, calcular corretamente infoHash v1 e/ou v2, e expor a
árvore de arquivos (`file tree`) e as camadas de hash (`piece layers`) em
memória. Ainda não baixa nem verifica peças pela rede.

- [x] `TorrentAttribute.h`: adicionar `infoHashV2` (32 bytes), `metaVersion`
  (1 ou 2), `v2FileEntries` (path + length + piecesRoot por arquivo,
  seguindo a ordem de travessia do `file tree`), `pieceLayers` (map
  piecesRoot(32 bytes) → hashes SHA-256 concatenados daquele arquivo).
- [x] `bittorrent_helper.cc`: detectar `info["meta version"] == 2`;
  quando presente, parsear `info["file tree"]` (dict recursivo de
  segmentos de path; folha é uma entrada com chave `""` contendo
  `{length, pieces root}`) e `info["piece layers"]`; calcular infoHashV2
  em cima do mesmo `encodedInfoDict` já usado pro v1.
- [x] `parseMagnet()`: aceitar `xt=urn:btmh:1220<hex>` além de
  `xt=urn:btih:<hex>`; um magnet híbrido pode ter os dois `xt=` na mesma
  URI — nesse caso guardar os dois hashes.
- [x] `torrent2Magnet()` / `metadata2Torrent()`: atualizar para
  incluir/aceitar o hash v2 quando presente (compatibilidade de saída).
- [x] Testes (CppUnit, mesmo padrão de `BittorrentHelperTest.cc`): torrent
  v2-only sintético pequeno, torrent híbrido sintético pequeno, magnet v2
  e magnet híbrido — conferir infoHash v1 (quando aplicável), infoHash v2,
  file tree e piece layers batendo com valores calculados à mão.
- [x] `make check` passando (suite inteira, não só os testes novos — pra
  garantir que v1 não quebrou). Os dois testes LPD de multicast só executam
  quando existe interface IPv4 multicast ativa; neste contêiner há apenas
  loopback, portanto não exercitam envio/recebimento de multicast aqui.
  Os demais 981 casos CppUnit passaram, incluindo os novos vetores reais
  do libtorrent para v2-only e híbrido e os testes v1 existentes.
  Downloads v2-only são recusados com erro explícito até a integração das
  fases seguintes; híbridos ainda trafegam somente no caminho v1.

### Fase 2 — Verificação de integridade por árvore de merkle
Objetivo: dado o conteúdo de uma peça (16 KiB por bloco, conforme BEP 52),
calcular a árvore de merkle SHA-256 e validar contra `piecesRoot`
(arquivos de 1 peça) ou contra a camada correspondente em `pieceLayers`
(arquivos maiores). Ainda uma função pura, testável isoladamente, sem
integrar no motor de download.

- [ ] Função utilitária de merkle tree (bottom-up, blocos de 16 KiB,
  padding com hash de bloco zerado quando o arquivo não é múltiplo exato —
  regra exata do BEP 52 para isso).
- [ ] Tratamento de "padding files" entre arquivos no `file tree` (BEP 52
  exige alinhamento de peça entre arquivos; verificar como o v1 já lida
  com fronteiras de arquivo em `PieceedSegment`/`FileEntry` pra reusar).
- [ ] Testes com vetores conhecidos (torrent de teste real, ou vetores de
  teste da spec/libtorrent, pra garantir interoperabilidade correta desde
  o início — não inventar formato próprio).

### Fase 3 — Integração com armazenamento e verificação ao vivo
Objetivo: um torrent v2/híbrido pode ser adicionado, as peças recebidas
pela rede (Fase 4) são verificadas com SHA-256/merkle em vez de SHA-1, e
`DownloadContext`/`PieceStorage` sabem lidar com os dois modos no mesmo
processo (importante pro híbrido, que fala com peers v1 e v2
simultaneamente).

- [ ] `Piece.cc` / `PieceStorage` / `PieceHashCheckIntegrityEntry`: tornar
  o algoritmo de verificação de peça plugável por torrent (v1=SHA-1 sobre
  peça inteira, v2=merkle SHA-256), sem quebrar o caminho v1 existente.
- [ ] Mapear `v2FileEntries` para os `FileEntry` já usados pelo resto do
  motor (reaproveitar ao máximo a infraestrutura de arquivos existente).

### Fase 4 — Protocolo peer-wire (BEP 52)
Objetivo: aria2-ultra fala v2 de verdade com outros peers.

- [ ] Handshake: torrents v2-only anunciam um info-hash diferente do v1 no
  handshake padrão (regra exata do BEP 52 — conferir spec); híbrido
  precisa responder corretamente pros dois tipos de peer.
- [ ] Novas mensagens de extensão (BEP 10 extended protocol): `hash
  request`, `hashes`, `hash reject` — pedido/resposta de camadas da árvore
  de merkle entre peers. Implementar como novas classes `Bt*Message`
  seguindo o padrão das existentes (`BtExtendedMessage.*` como referência).
- [ ] Metadata exchange (`ut_metadata`, BEP 9) para magnet v2/híbrido:
  entregar o `info` dict completo (incluindo `file tree` e `piece
  layers`) via peers, não só via `.torrent` local.

### Fase 5 — Tracker e DHT
- [ ] Anúncio a tracker HTTP/UDP com o info-hash certo (v1 e/ou v2,
  conforme o torrent).
- [ ] DHT (BEP 52 estende BEP 5): mesmo protocolo Kademlia, trocando qual
  info-hash é usado nas queries `get_peers`/`announce_peer` pro v2.

### Fase 6 — Validação end-to-end
- [ ] Baixar torrents v2 e híbridos reais (não sintéticos) e conferir
  interoperabilidade com um cliente de referência (libtorrent/qBittorrent
  como "peer conhecido bom").
- [ ] Rodar a suite de regressão completa do v1 (`make check`) e o teste
  de download direto — confirmar que nada regrediu.
- [ ] Só depois disso, atualizar `teste-torrent-v1-diagnostico.cmd`
  (ou criar `teste-torrent-v2-diagnostico.cmd`) pra cobrir o fluxo v2
  real, e compilar `aria2c.exe` novo pra Windows com tudo isso dentro.

## Estado atual

- Fase 0: concluída.
- Fase 1: parsing e hashes implementados e validados em Linux; downloads
  v2-only aguardam integridade e protocolo. Veja observação dos testes LPD.
- Fases 2-6: não iniciadas.

## Próximo passo concreto

Implementar a Fase 2: utilitário de Merkle puro, com vetores de referência
e validação de `piece layers`, antes de integrar a verificação ao download.
