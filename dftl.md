2. DFTL 的具体架构
DFTL 采用页级映射（Page-Level Mapping），但由于 SRAM 资源有限，完整的地址映射表存储在闪存的翻译页（Translation Pages）中，仅将热点地址映射加载到 SRAM 中。

2.1 数据存储结构
DFTL 将闪存块分为两类：

数据块（Data Blocks）：存储真实的用户数据。
翻译块（Translation Blocks）：专门用于存储逻辑到物理地址映射信息。
每个翻译页（Translation Page）存储多个逻辑页到物理页的映射条目，这些翻译页存储在翻译块中。

2.2 数据结构
全局映射表（Global Mapping Table, GMT）：

存储所有逻辑页号（LPN）到物理页号（PPN）的映射信息。
由于映射条目较多，不直接存储在 SRAM，而是放在闪存的翻译块中。
缓存映射表（Cached Mapping Table, CMT）：

仅存储最近访问的页级映射条目。
通过LRU（Least Recently Used）算法管理缓存替换。
全局翻译目录（Global Translation Directory, GTD）：

记录 GMT 在翻译页中的存储位置，使得需要时可以快速从闪存加载映射条目。
3. 读写操作的流程
3.1 读操作
先在 SRAM 中的 CMT 查找逻辑页号（LPN）对应的物理页号（PPN）。
命中（Hit）：直接读取 PPN 指定的物理页数据。
未命中（Miss）：
从 GTD 获取 LPN 在 GMT 中的位置。
从闪存的翻译页中加载映射条目到 CMT。
将 LPN 对应的 PPN 记录到 CMT，并进行数据读取。
3.2 写操作
先在 CMT 中查找 LPN 对应的 PPN。
命中：
选择一个新的物理页写入数据（避免覆盖原数据）。
更新 CMT，并标记原来的物理页为无效。
未命中：
先从闪存的翻译页加载映射条目到 CMT。
进行数据写入，并更新映射条目。
3.3 地址映射缓存管理
DFTL 采用 分段 LRU（Segmented LRU）算法 来管理 CMT：

热点数据（Hot Data）：优先保留在 SRAM 中，避免频繁的地址查找开销。
冷数据（Cold Data）：在 CMT 空间不足时被淘汰，并回写到 GMT 中。
4. 垃圾回收（GC）优化
4.1 传统 FTL 的问题
传统的混合 FTL 方案（如 FAST）在随机写入密集的负载下，容易触发全合并（Full Merge），导致：

需要移动大量有效页，增加写放大（Write Amplification）。
造成额外的闪存擦除和写入，降低 SSD 寿命。
4.2 DFTL 的垃圾回收策略
完全避免全合并：由于 DFTL 采用页级映射，所有数据块都可直接更新，无需合并日志块。
智能选择回收块：
优先回收“冷数据”块，避免影响系统性能。
采用**懒惰更新（Lazy Updates）**策略，减少无效数据的频繁搬移。
4.3 进一步优化
批量更新（Batch Updates）：

当多个 CMT 记录需要更新时，批量合并写入闪存。
避免频繁的小规模翻译页写入，提高写入效率。
减少翻译页的读写：

只在必要时更新翻译页，降低翻译开销。
使用 GTD 快速查找翻译页的位置，提高查询效率。
