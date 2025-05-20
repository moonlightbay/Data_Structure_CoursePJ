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

# # 使用示例
# gml_path = Path("path/to/dataset")
# for gml_file in gml_path.glob("*.gml"):
#     add_multigraph_attribute(gml_file)