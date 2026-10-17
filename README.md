# ItchBook C++

一个用于 C++ Quant Developer 作品集的 **Nasdaq TotalView-ITCH 5.0 行情重构**项目。它从离线二进制消息还原可展示订单簿，不接单、不撮合、不模拟交易所。项目按协议正确性 → 基准正确性 → 性能优化的顺序开发。

当前为 **M1 最小可运行版**：读取 `uint16` 大端长度前缀的消息文件，仅接受 A（新增）和 D（删除）消息；按股票代码分别维护订单、价位聚合量与最优买卖价。遇到其他消息、截断、重复订单或错误引用会停止并报错，因此不能用当前版本重放完整 ITCH 日文件。输入格式是本项目的离线容器约定；ITCH 消息本体的字段以 Nasdaq 规范为准。

## 快速运行

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j2
ctest --test-dir build --output-on-failure
./build/itchbook path/to/length-prefixed-A-D.bin
```

程序输出每个出现过的 symbol 的最终 `bid`、`ask`、对应聚合股数和消息总数；价格输出为精确的四位小数。不提供内置真实行情数据。

## 读源码的顺序

优先学习 [CppTrader](https://github.com/chronoxor/CppTrader)：先读 `itch_handler.h/.inl` 理解消息解码，再读 `market_manager.h/.inl` 和 `order_book.h` 理解订单引用及状态维护，最后读其 `performance/` 中的测量入口。本项目是独立实现，不复制上游代码。第二阶段再对照 [Liquibook](https://github.com/enewhuis/liquibook) 的订单生命周期；它是撮合引擎，职责和本项目不同。QuickFIX 与 QuantLib 暂不在这条行情重构主线上。

设计、协议与失败语义见 [架构](docs/ARCHITECTURE.md)、[协议覆盖](docs/PROTOCOL.md)；逐步实现和验收见 [路线](docs/ROADMAP.md)；比较方法见 [基准方案](docs/BENCHMARK.md)；证据和许可见 [来源](docs/SOURCES.md)。

## 作品集目标

最终提供完整可重放的订单生命周期、按 symbol 的逐消息 BBO 校验、参考实现差分测试、带安全回退的优化索引，以及可复现的同机基准报告。**目前只完成 M1，不声称上述终态已经实现或有任何延迟优势。**
