重要！！！ 考虑到原始 Topology Zoo 数据集未进行过数据清洗，在解析 GML 文件前/时需要考虑数据清洗：

节点 node 缺失经纬度信息（Longitude 和 Latitude）：若一条链路的两端节点，至少有一个节点缺失经纬度信息，则默认该链路的时延为 5 毫秒。

某个 GML 文件存在重复的链路（source 
→ target 相同）：若直接用 nx.read_gml() 读取该文件，会报错：

networkx.exception.NetworkXError: edge xxx duplicated
建议手动在 GML 文件的 graph 块中添加属性 multigraph 1，表示该图允许多重边，并在使用 nx.read_gml() 读取后，将多余的边删除，例如：

graph [
    multigraph 1  // 允许重复边，确保不会报错
    DateObtained "22/10/10"
    GeoLocation "USA"
    ...
]
也可以写一个自动脚本来为每个 GML 文件添加 multigraph 1 属性：

import re
from pathlib import Path
def add_multigraph_attribute(gml_file):
    with open(gml_file, 'r') as file:
        content = file.readlines()
    
    # 在 graph 块中添加 multigraph 1 属性
    for i, line in enumerate(content):
        if line.startswith("graph ["):
            content.insert(i + 1, "    multigraph 1\n")
            break
    
    # 将修改后的内容写回文件（记得备份原始文件）
    with open(gml_file, 'w') as file:
        file.writelines(content)
# 使用示例
gml_path = Path("path/to/dataset")
for gml_file in gml_path.glob("*.gml"):
    add_multigraph_attribute(gml_file)