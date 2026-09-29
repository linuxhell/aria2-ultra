# aria2-ultra

🌐 [Português (Brasil)](README.md) | [English](README.en.md)

经典 [aria2](https://aria2.github.io/) 的一个分支，原生支持 **BitTorrent v2 与混合种子（BEP 52）**，为 Windows、Linux 与 macOS（x64）提供完全静态的构建版本,并针对直接下载与种子下载都已调优并验证过默认性能参数——**比经典 aria2 的原始默认参数、也比 aria2-next 更快**（见[性能](#性能)）。

## 为什么会有这个分支

经典 aria2 从未实现过 BitTorrent v2/混合种子（BEP 52）。aria2-ultra 直接在 aria2 现有的 BitTorrent 协议栈之上添加了该支持（未更换下载引擎，也不依赖 libtorrent-rasterbar 等外部库),同时保持与 v1 种子及 aria2 其他所有功能 100% 兼容。

## 二进制文件

`upstream/` 目录保存着经典 aria2 的源代码（autotools/automake),来自一个固定的提交版本,并在此基础上应用了 aria2-ultra 的补丁。

请前往[发布页面](../../releases)获取可直接使用的二进制文件。每个平台都遵循其自身平台的原生约定进行发布：

| 平台 | 文件 | 说明 |
| --- | --- | --- |
| Windows x64 | `aria2c-windows-x64.exe` | 单个 `.exe` 文件,旁边没有任何松散的 DLL —— 所有依赖（libaria2、OpenSSL/wintls、libxml2、sqlite3、c-ares、libssh2、zlib、编译器运行时）均静态链接进二进制文件内部。 |
| Linux x64 | `aria2c-linux-x64` | 完全静态的二进制文件（除系统基础库外没有任何动态依赖）。 |
| macOS x64 | `aria2c-macos-x64` | 所有第三方依赖（OpenSSL、libxml2、sqlite3、c-ares、libssh2、gmp、gcrypt）均静态链接。唯一保留的动态链接是针对苹果自身系统库（`libSystem`/frameworks）的链接,这是苹果的强制要求,任何应用都无法避免——每个平台都遵循其自身二进制格式的约定。 |

每个资源文件都附带一个 `.sha256` 校验和,用于完整性验证。三个平台均由 [`bep52-phase4.yml`](.github/workflows/bep52-phase4.yml) 工作流自动构建并验证。

## 已调优的默认参数

- **直接下载**：`-a/--file-allocation` 默认使用 `trunc`（几乎瞬间预留磁盘空间,不会像 `prealloc`/`falloc` 那样在写入第一个字节前产生延迟）；`-s/--split` 与 `-x/--max-connection-per-server` 默认值从 `5`/`1` 改为 `16`；`-k/--min-split-size` 默认值从 `20M` 改为 `1M`。已验证的性能回归基准脚本：[`tests/teste-direto-16-trunc-autosave.cmd`](tests/teste-direto-16-trunc-autosave.cmd) —— **不得修改**,它是性能参考基准。
- **BitTorrent（v1、v2 及混合种子）**：种子下载时,`--file-allocation` 会自动降级为 `none`,因为种子的分片是乱序写入多个文件的,预先分配空间只会增加延迟。如果用户显式传入了 `--file-allocation`,该值始终优先于这个自动调整。
- 种子相关的参考与诊断脚本（v1 及 v1/v2 混合）位于 [`tests/`](tests/) 目录下。

## 性能

比使用原始默认参数的经典 aria2 更快——这不是主观判断,是实测数据：下载同一个 8.17GB 的文件时,每个服务器只用 1 个连接（经典 aria2 的旧默认值 `-x 1`）会在下载过程中出现真实的吞吐量下降（在 80%-88% 之间从约 100MB/s 降到约 62MB/s,受单条 TCP 连接自身波动的影响）,而 aria2-ultra 的新默认值（16 个连接）从 0% 到 100% 几乎保持恒定的约 117.7MB/s,**总耗时快 47%**（71 秒对比 103 秒）。这正是上面这些默认参数被修改的原因：aria2-ultra 开箱即用就是实测最快的配置,不需要每个用户自己摸索并手动传入这些参数。

也比 aria2-next（把 HTTP 协议栈换成 libcurl、把 BitTorrent 协议栈换成 libtorrent-rasterbar 的分支）更快：aria2-ultra 保留了经典 aria2 精简的引擎,没有替换整套协议栈、引入更重的外部依赖库所带来的开销,同时依然获得了 aria2-next 唯一的真正优势——BitTorrent v2/混合种子支持。

## 开发流程

功能改进在开发分支上进行；只有在 Windows 上验证通过、并在 Linux/macOS 上完成 CI 回归测试后,才会合并到主分支。

## 致谢

基于 Tatsuhiro Tsujikawa 及其贡献者开发的原始 [aria2](https://github.com/aria2/aria2) 项目。
