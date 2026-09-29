# Changelog

🌐 [Português (Brasil)](CHANGELOG.md) | [简体中文](CHANGELOG.zh-CN.md)

All notable changes in aria2-ultra relative to classic aria2 are documented here.

## [Unreleased]

### Added
- Native BitTorrent v2 and hybrid (BEP 52) support on top of classic aria2's existing BitTorrent stack: v2 metadata/magnet parsing, Merkle-root and piece-layer integrity verification, and the correct 20-byte infohash across protocol paths (handshake, tracker, DHT, peer-wire).
- Official fully static builds for Windows, Linux, and macOS (x64), published via CI (`.github/workflows/bep52-phase4.yml`), with no third-party DLL/shared library next to the binary.

### Changed
- `-a/--file-allocation` now defaults to `trunc` for direct downloads (previously `prealloc`), eliminating the file allocation/zero-fill delay before the first byte is written.
- BitTorrent downloads (v1, v2, and hybrid) automatically downgrade `--file-allocation` to `none`, since pieces are written out of order across potentially many files — upfront allocation doesn't help there. An explicit value passed by the user always takes precedence.

### Removed
- The versioned reference `aria2c.exe` binary and `SHA256SUMS.txt` that lived at the repository root — binaries are now only distributed as Release assets, never committed.
- Documentation and test scripts for the abandoned aria2-next/libtorrent-rasterbar approach (the current fork stays on the classic aria2 base).
- Internal planning and handoff notes (`BT-V2-PLANO.md`, `HANDOFF-CHATGPT.md`) that made no sense outside the development process.

## Base

Vendored from classic aria2 (`aria2/aria2`), with the patches above applied on top.
