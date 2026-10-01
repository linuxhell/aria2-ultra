## Português (Brasil)
- Distribui conexões HTTP(S) segmentadas entre os endereços resolvidos do servidor, quando há vários IPs e nenhum proxy.
- Preserva as correções BEP52 para torrents v2/híbridos e os padrões de 16 conexões, divisão mínima de 1 MiB e alocação por truncamento em downloads diretos.
- Binários x64 independentes para Windows, Linux e macOS, compilados do mesmo commit e publicados apenas após aprovação do CI Linux.
- No ADM, atualize o aria2 em **Ferramentas** após a publicação.
- Multipath QUIC, MARS, RFC 9842 e MoQT não fazem parte deste motor C++; são integrações experimentais separadas do ADM.

## English
- Distributes segmented HTTP(S) connections across resolved server addresses when multiple IPs are available and no proxy is configured.
- Preserves BEP52 v2/hybrid fixes and the 16-connection, 1 MiB minimum split, truncating allocation defaults for direct downloads.
- Standalone Windows, Linux and macOS x64 binaries built from the same commit and published only after Linux CI succeeds.
- Update aria2 under **Tools** in ADM after publication.
- Multipath QUIC, MARS, RFC 9842 and MoQT are separate experimental ADM integrations, not features of this C++ engine.

## 简体中文
- 在服务器解析出多个 IP 且未配置代理时，将 HTTP(S) 分段连接分配到不同地址。
- 保留 BEP52 v2/混合种子修复及直链下载默认配置：16 个连接、1 MiB 最小分段和截断式文件分配。
- Windows、Linux 和 macOS x64 独立程序从同一提交构建，仅在 Linux CI 成功后发布。
- 发布后，请在 ADM 的 **工具** 中更新 aria2。
- Multipath QUIC、MARS、RFC 9842 和 MoQT 属于独立的 ADM 实验集成，并非本 C++ 引擎的功能。
