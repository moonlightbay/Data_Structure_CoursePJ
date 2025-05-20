# Data_Structure_CoursePJ

题目背景
随着互联网用户数量的激增和数据流量的不断扩展，运营商和企业级网络正面临着前所未有的挑战。为了实现高可靠性和低延迟的数据传输，网络运营商通常构建复杂的骨干网络和数据中心集群，其核心组成部分为各类路由器、交换机及边缘节点，这些网络设备通过互联形成一个庞大的图结构。
在实际运行过程中，网络环境会受到各种因素的影响，例如：

节点故障与启停：网络中的某些节点（如核心路由器或数据中心的交换机）可能由于硬件老化、软件异常、计划内维护或突发故障而被禁用或失效，出现动态的关停或启动现象，影响自身与附近节点的链路通信。

动态负载波动：网络中的链路负载并非静态。在高峰时段或出现突发流量时，某些链路的负载会急剧上升，导致延迟增加或带宽受限；而在低谷时段，负载降低则可能使得某些链路延迟或消耗减小，从而为路径优化提供机会。

传统的路由协议（例如 OSPF）在遇到节点故障或负载突变时，往往需要全局重新计算最短路径树，响应较慢且资源消耗大。为此，本项目要求设计一个动态路由系统，在动态波动的网络环境下，能够迅速计算受影响子网的最短路由，从而提升整体网络的鲁棒性和传输性能。

为了模拟真实的网络拓扑，本项目使用 Internet Topology Zoo 数据集（https://github.com/afourmy/3D-internet-zoo）。Internet Topology Zoo 是一个持续进行的项目，旨在收集全球各地真实的网络拓扑数据。这些拓扑数据由各大运营商提供或从公开网络图中人工整理得来，涵盖了互联网上各种类型的网络（例如企业网络、学术网络、运营商骨干网等）。Internet Topology Zoo 更详细的介绍请参阅 The Internet Topology Zoo.pdf。

为了统一数据集格式，本次采用 3D-internet-zoo 项目提供的 GML 格式 Internet Topolgy Zoo 数据集。

全局属性：文件头部定义诸如网络名称、采集日期、地理位置、数据来源等信息。
graph [
    DateObtained "22/10/10"
    GeoLocation "USA"
    GeoExtent "Country"
    Network "US Signal"
    Provenance "Primary"
    Note "Fiber provider"
    Source "http://www.ussignalcom.com/network"
    ...
]
对于网络的全局属性，我们只关注 Network 项（UsSignal.gml 的 Network 项的值为 Us Signal），作为该网络的唯一名称。




节点定义：使用 node [ ... ] 块来描述网络节点，每个节点通常包含唯一的 id、label（名称）、Longitude 经度、Latitude 维度等其它属性。例如：

node [
    id 0
    label "Indianapolis"
    Country "United States"
    Longitude -86.15804
    Latitude 39.76838
]
...
node [
    id 11
    label "Lafayette"
    Country "United States"
    Longitude -86.87529
    Internal 1
    Latitude 40.4167
]
对于节点属性，我们只关注 id Longitude Latitude 3个项：
id：节点唯一标识
Longitude，Latitude：节点地理位置的经度、纬度，用于计算相连节点间的球面距离、并用于计算链路时延




链路定义：使用 edge [ ... ] 块来描述 source 节点与 target 节点间的一条无向链路。例如：
edge [
    source 0
    target 11
    id "e18"  // 链路唯一标识
    ...  // 其它标签属性
]  // 代表 节点 id=0 <-> 节点 id=11 的一条无向链路

对于链路属性，我们只关注 source 和 target 2个项，分别表示链路的起始节点和终止节点。由于链路的标签属性并不统一，故不在本项目考虑范围内。
注：source A, target B 与 source B, target A 代表同一条无向链路。



由于 GML 格式解析较为繁琐，推荐使用 Python 解析 GML 并转换为 C/C++ 易于处理的格式。利用 Python 内置的 NetworkX 库可以方便地读取 GML 文件。NetworkX 内部提供了 read_gml() 函数来解析 GML 文件，解析后再将图数据存储成如 CSV、文本边列表等格式，便于 C/C++ 程序加载。