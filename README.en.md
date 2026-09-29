# aria2-ultra

🌐 [Português (Brasil)](README.md) | [简体中文](README.zh-CN.md)

A fork of classic [aria2](https://aria2.github.io/) with native **BitTorrent v2 and hybrid (BEP 52) support**, fully static builds for Windows, Linux, and macOS (x64), and performance defaults already tuned and validated for both direct downloads and torrents.

## Why this fork exists

Classic aria2 never implemented BitTorrent v2/hybrid (BEP 52). aria2-ultra adds that support directly on top of aria2's existing BitTorrent stack (no engine swap, no dependency on external libraries such as libtorrent-rasterbar), while staying 100% compatible with v1 torrents and every other aria2 feature.

## Binaries

The `upstream/` directory holds classic aria2's source code (autotools/automake), vendored from a pinned commit, with aria2-ultra's patches applied on top.

Grab ready-to-use binaries from the [Releases page](../../releases). Each platform is published following its own native conventions:

| Platform | File | Notes |
| --- | --- | --- |
| Windows x64 | `aria2c-windows-x64.exe` | A single `.exe` with no loose DLLs next to it — everything (libaria2, OpenSSL/wintls, libxml2, sqlite3, c-ares, libssh2, zlib, compiler runtime) statically linked into the binary. |
| Linux x64 | `aria2c-linux-x64` | Fully static binary (no dynamic dependencies beyond the base system libraries). |
| macOS x64 | `aria2c-macos-x64` | All third-party dependencies (OpenSSL, libxml2, sqlite3, c-ares, libssh2, gmp, gcrypt) statically linked. The only remaining dynamic link is against Apple's own system libraries (`libSystem`/frameworks), which Apple requires and no app can avoid — each platform follows its own binary format's conventions. |

Every asset ships with a `.sha256` checksum for integrity verification. All three platforms are built and verified automatically by the [`bep52-phase4.yml`](.github/workflows/bep52-phase4.yml) workflow.

## Tuned defaults

- **Direct downloads**: `-a/--file-allocation` defaults to `trunc` (reserves disk space almost instantly, without the delay `prealloc`/`falloc` add before the first byte is written). Validated regression baseline: [`tests/teste-direto-16-trunc-autosave.cmd`](tests/teste-direto-16-trunc-autosave.cmd) — **must not be changed**, it's the performance reference.
- **BitTorrent (v1, v2, and hybrid)**: the same `--file-allocation` is automatically downgraded to `none` for torrent downloads, since pieces are written out of order across potentially many files and upfront allocation only adds delay. An explicit `--file-allocation` passed by the user always overrides this automatic adjustment.
- Reference and diagnostic torrent scripts (v1 and mixed v1/v2) live in [`tests/`](tests/).

## Development

Improvements happen on a development branch; merges into the main branch only happen after validation on Windows and regression checks on Linux/macOS via CI.

## Credits

Based on the original [aria2](https://github.com/aria2/aria2), by Tatsuhiro Tsujikawa and contributors.
