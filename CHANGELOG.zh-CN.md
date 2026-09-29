# 更新日志

🌐 [Português (Brasil)](CHANGELOG.md) | [English](CHANGELOG.en.md)

本文档记录了 aria2-ultra 相对于经典 aria2 的所有重要变更。

## [未发布]

### 新增
- 在经典 aria2 现有的 BitTorrent 协议栈之上原生支持 BitTorrent v2 与混合种子（BEP 52）：v2 元数据/磁力链接解析、基于 Merkle root 与 piece layers 的完整性校验,以及在协议各环节（握手、tracker、DHT、peer-wire）中使用正确的 20 字节 infohash。
- 通过 CI（`.github/workflows/bep52-phase4.yml`）发布的 Windows、Linux 与 macOS（x64）官方完全静态构建版本,二进制文件旁不附带任何第三方 DLL / 动态库。

### 变更
- 直接下载时 `-a/--file-allocation` 默认改为 `trunc`（此前为 `prealloc`),消除了写入第一个字节前的文件分配/清零延迟。
- BitTorrent 下载（v1、v2 及混合种子）会自动将 `--file-allocation` 降级为 `none`,因为分片是乱序写入多个文件的,预先分配空间毫无帮助。如果用户显式传入了该参数,则始终优先使用用户指定的值。

### 移除
- 此前提交在仓库根目录的参考二进制文件 `aria2c.exe` 与 `SHA256SUMS.txt`——现在二进制文件只作为 Release 资源分发,不再提交到仓库中。
- 已废弃的 aria2-next/libtorrent-rasterbar 方案相关的文档与测试脚本（当前分支继续基于经典 aria2）。
- 与开发流程本身相关、对外部无意义的内部规划与交接笔记（`BT-V2-PLANO.md`、`HANDOFF-CHATGPT.md`）。

## 基础版本

基于经典 aria2（`aria2/aria2`）vendored 而来,并在此基础上应用了上述补丁。
