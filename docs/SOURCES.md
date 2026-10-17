# 来源与许可

- [Nasdaq TotalView-ITCH 5.0 规范](https://classic.nasdaqtrader.com/content/technicalsupport/specifications/dataproducts/NQTVITCHSpecification.pdf)：字段、长度、消息语义的唯一规范依据；阅读日期 2026-10-17。
- [Nasdaq ITCH 示例数据入口](https://emi.nasdaq.com/ITCH/Nasdaq%20ITCH/)：后续真实样本来源，M1 未下载/验证整日数据。
- [CppTrader](https://github.com/chronoxor/CppTrader)，本地 `Areas/Quant/upstream/CppTrader`，提交 `39421f50ae673be34235e24eac95827220763652`，MIT；参考 `include/trader/providers/nasdaq/itch_handler.h`、`include/trader/matching/market_manager.h`、`performance/`，没有复制源码。仓库 README 中的旧机器数字不是本项目测量结果。
- [Liquibook](https://github.com/enewhuis/liquibook)，本地 `Areas/Quant/upstream/liquibook`，提交 `2427613b32f1667abae68a01df6af9ba8270f8e7`；仅作为后续撮合/订单模型补充读物。

项目代码与测试独立编写，按本仓库 [MIT License](../LICENSE) 发行。任何新增第三方代码或数据都需单独核对许可并注明出处。
