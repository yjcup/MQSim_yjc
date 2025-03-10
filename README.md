## FTL

SSD特性：	

1. 擦除写
2. 每个闪存块都是有寿命的
3. 存在读干扰，读的太多，电子会到浮珊层，到时`1`变`0`
4. 存在数据保持问题，电荷会流失
5. 存在坏块

针对优化 FTL 的管理与调度来提高闪存设备性能,  现有工作主要分为以下四类:

1. **通过减少写放大与尾延迟**

   **核心问题：写放大和尾延迟**

   - **写放大:** 由于闪存的擦除操作是以块为单位进行的，即使只修改一小部分数据，也需要先将整个块的数据读取出来，修改后再写入。这导致实际写入的数据量大于用户请求写入的数据量，这个放大倍数就是写放大。写放大过大会降低SSD的写入速度和寿命。
   - **尾延迟:** 指的是完成一个操作所需的最长时间。在SSD中，某些操作（例如垃圾回收）可能需要较长时间，导致性能波动，出现偶尔的延迟尖峰，这就是尾延迟。过高的尾延迟会影响系统的实时性和响应速度。

   **两种优化方案：无冗余和有冗余**

   文章提出了两种优化FTL性能的方案，根据是否使用冗余的闪存阵列进行分类：

   1. **无冗余方案:** 这类方案的核心思想是减少写入的数据量，从而降低写放大和GC（垃圾回收）的负担。主要技术包括：
      - **编码、压缩、去重:** 通过减少数据冗余，降低实际写入的数据量。
      - **优化缓存技术:** 将频繁访问的数据缓存在DRAM中，减少对闪存的访问次数。
      - **优化GC:** 改进垃圾回收算法，减少数据迁移量和GC的执行时间，从而降低写放大和尾延迟。
   2. **有冗余方案 (RAIN):** RAIN（Redundant Array of Independent NAND）类似于RAID（Redundant Array of Independent Disks），通过冗余的闪存芯片提供数据保护功能。这类方案的优化重点在于：
      - **条带设计:** 优化数据在多个闪存芯片上的分布，提高并行写入性能。
      - **减少校验开销:** 校验数据是为了保证数据一致性，但会增加写入开销。因此，需要优化校验算法，减少校验数据的写入量。

2. **通过提升并行性**

3. **通过利用不同闪存页特性优化性能**

4. **通过优化校验方案提升性能**(这个就不看了，重点看上面三个)

### SSD的多层并行性

![image-20241112160439465](https://online-education02.oss-cn-hangzhou.aliyuncs.com/blog/202411121604819.png)

![image-20241113095942093](https://online-education02.oss-cn-hangzhou.aliyuncs.com/blog/202411130959260.png)

闪存芯片由五个层次组成： 芯片（chip）—晶圆（die）—分组（plane）—块（block）—页（page）。固态盘有四个层次的并行结构：通道间并行—芯片间并行—晶圆间并行—分组间并行。有效利用这四个层次的并行是提 高固态盘整体读写性能的关键。

### 内存映射

1. page-level:粒度很细，来的每一个page都能随便映射到SSD中的每一块，但是所占的SRAM太大了。For example, a 16GB flash memory requires approximately 32MB of SRAM space for storing a page-level mapping table。

2. block-level:太不灵活了，只要LBN的block字段不一样，都要重新开一个block来存储数据。导致数据存储非常离散，非常不利于垃圾回收。

3. hybrid mapping:they share one fundamental design principle. All of these schemes are a hybrid between page-level and blocklevel schemes. They logically partition their blocks into two groups - Data Blocks and Log/Update Blocks. Data blocks form the majority and are mapped using the block-level mapping scheme. A second special type of blocks are called log blocks whose pages are mapped using a page-level mapping style. 

   ![image-20241114190833421](https://online-education02.oss-cn-hangzhou.aliyuncs.com/blog/202411141908512.png)

      那么为了解决块映射的问题，所有要写的请求都写到页级映射的日志块中，当日志块写满了都给写到数据块当中，很好的解决了块映射写离散的

      问题，但是将日志快和数据块合并的操作还是非常费时。

   ​    ![image-20241114193059306](https://online-education02.oss-cn-hangzhou.aliyuncs.com/blog/202411141930358.png)

   ​	

   #### 事务

   在SSD架构中，**事务（Transaction）**是指将多个SSD提供的命令组合在一起，由控制器在闪存内部执行的一个原子操作单元。

   1. 逻辑请求分解：
      - 用户提交的I/O请求首先被分解为多个更细粒度的子请求。例如，1个I/O写请求可能涉及多个页或块。
   2. 调度器分组：
      - 调度器将这些子请求分配到不同的资源（如芯片、平面），以构建事务。
      - 高效的事务需要同时考虑时间本地性（请求到达时间接近）和空间本地性（分配到不同资源）。
   3. 执行事务：
      - 一旦构建完成，事务会被提交到闪存设备执行，通常会利用多通道、多裸片、多平面等硬件特性，尽量让事务内部的子操作并行执行。

   #### DFTL

   #### 处理流程

   1. void TSU_OutOfOrder::Schedule()

      在调度之前会将各个transations挂到对应的channel和die上，之后开始遍历所有的空闲通道的chip,逐个处理

   2. process_chip_requests(chip);

      开始处理，这里是芯片级，如果chip不是free还要操作，这个先跳过，分别把两个优先级队列赋值

   3. bool TSU_Base::issue_command_to_chip(）

      这个是die晶圆级，遍历所有die，找出可以在同一个die上运行的（多平面操作），

   4. _NVMController->Send_command_to_chip(transaction_dispatch_slots);

      发送命令到die，就是注册一个回调函数

      ```c++
      			Simulator->Register_sim_event(Simulator->Time() + suspendTime + target_channel->ProgramCommandTime[transaction_list.size()] + data_transfer_time,
      							this, dieBKE, (int)NVDDR2_SimEventType::PROGRAM_CMD_ADDR_DATA_TRANSFERRED);
      ```
#### 一个写请求的处理过程

1. 应用将请求发送到主机端队列，并通过PCIe总线通知ssd
2. ssd更新队列请求主机命令
3. 主机端读取命令在将命令传递回去
4. ssd开始处理请求
5. ftl翻译，缓存未命中去取

```shell
******************************
Executing scenario 1 out of 1 .......
Investigating input trace file: ./test.trace
Trace file: ./test.trace seems healthy
RegisterEvent 946244000 0x5555786ca9a0  //Submit_io_request(request); 发送请求

RegisterEvent 946244008 0x5555786ca2f0 //device更新队列
RegisterEvent 946244016 0x5555786ca2f0 //抓取请求
RegisterEvent 946244041 0x5555786ca2f0 // 读取请求发送到主机侧并开始执行请求
Address mapping table query - Stream ID:0, LPA:22863473, MISS 
0: 946244041 Issueing Transaction - Type:Read, , PPA:45613442, LPA:18446744073709551615, Channel: 5, Chip: 1
Chip 5, 1, 1: Sending read command to chip for LPA: 18446744073709551615
RegisterEvent 946244331 0x55557646c490
RegisterEvent 946319351 0x55556af98f70
Command execution started on channel: 5 chip: 1
Channel 5 Chip 1- Finished executing read command
Chip 5, 1: finished  read command
RegisterEvent 946320887 0x55557646c490
Address mapping table insert entry - Stream ID:0, LPA:22863473, PPA:18446744073709551615
Address mapping table update entry - Stream ID:0, LPA:22863473, PPA:14155776
1: 946320887 Issueing Transaction - Type:Read, , PPA:14155776, LPA:22863473, Channel: 1, Chip: 2
// 之前缓存未命中不管 
Chip 1, 2, 1: Sending read command to chip for LPA: 22863473
RegisterEvent 946321177 0x55557646c490
RegisterEvent 946396197 0x55555b9045f0
Command execution started on channel: 1 chip: 2
Channel 1 Chip 2- Finished executing read command
Chip 1, 2: finished  read command
RegisterEvent 946397733 0x55557646c490
** Host Interface: Request #0 from stream #0 is finished
// 请求过来了显示处理cache相关，之后在将读取的数据发送到nvme中
RegisterEvent 946397889 0x5555786ca2f0
RegisterEvent 946397900 0x5555786ca2f0
[====================]  100% progress in Host.IO_Flow.Trace../test.trace

RegisterEvent 946397908 0x5555786ca2f0
MQSim finished at Mon Nov 25 07:46:47 2024

Total simulation time: 0:3:42
```

## 关于precondition
就是ssd初始化的时候会添加设置让ssd更加接近真实场景
主要目的:
模拟SSD在实际使用一段时间后的状态，而不是全新的空白状态
根据预设的稳态分布(steady state distribution)来初始化闪存块的使用情况
建立初始的逻辑地址(LPA)到物理地址(PPA)的映射关系

