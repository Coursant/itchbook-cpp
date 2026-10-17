# 协议契约与覆盖矩阵

依据 [Nasdaq TotalView-ITCH 5.0 官方规范](https://classic.nasdaqtrader.com/content/technicalsupport/specifications/dataproducts/NQTVITCHSpecification.pdf)。本项目的两字节长度前缀是 **文件容器约定**，不是 ITCH 消息本体字段，也不实现上层传输协议。所有多字节整数均大端；消息内 offset 从类型字节 0 开始。

| 类型 | 官方长度（字节） | 当前处理 | 对簿的作用 |
|---|---:|---|---|
| A | 36 | M1 已实现 | 新增可展示订单 |
| D | 19 | M1 已实现 | 删除剩余订单 |
| R | 39 | M2 计划 | symbol/locate 目录 |
| F | 40 | M2 计划 | 带 MPID 新增 |
| E | 31 | M2 计划 | 减少执行股数，归零即删除 |
| C | 36 | M2 计划 | 以成交价执行，簿仍按原展示价减量；打印标记另处理 |
| X | 23 | M2 计划 | 减少取消股数，归零即删除 |
| U | 35 | M2 计划 | 删除原 ID，用新 ID/数量/价格替换，保留 symbol/side |
| 其他 | 依官方类型 | 暂拒绝 | M2 分类后处理或明确忽略其非簿作用 |

M1 解码的 A 字段：`locate[1:3]`、`timestamp[5:11]`、`order_ref[11:19]`、`side[19]`、`shares[20:24]`、`stock[24:32]`、`price[32:36]`。D 使用 `locate[1:3]` 和 `order_ref[11:19]`。股票代码保留 ASCII 并去除**右侧空格**。跟踪号和时间戳当前不用于 BBO，但解码器会消耗完整报文；M2 应输出可审计事件元数据。

不变量：引用的订单必须存在、locate 与原订单一致；每个实时订单只属于一个 symbol；价位总量等于该价位所有剩余订单的数量之和；空买/卖边输出 `-`。完整规范中「Trade (P)」可能代表不可展示订单的交易，不应无条件修改可展示簿。E/C/X 的减量必须累计，达到零时删除。不能把 C 的成交价格当作原订单展示价。

后续用 [Nasdaq 样例数据入口](https://emi.nasdaq.com/ITCH/Nasdaq%20ITCH/) 获取合法样本；解压、会话分帧和授权条件先单独核实，再宣称支持整日回放。
