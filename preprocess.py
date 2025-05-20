import os
import networkx as nx
from pathlib import Path
import re
import math

# 添加multigraph属性来允许多重边
def add_multigraph_attribute(gml_file):
    with open(gml_file, 'r', encoding='utf-8') as file:
        content = file.readlines()
    
    # 检查是否已经有multigraph属性
    has_multigraph = False
    for line in content:
        if re.search(r'\s*multigraph\s+1', line):
            has_multigraph = True
            break
    
    # 如果没有multigraph属性，添加它
    if not has_multigraph:
        for i, line in enumerate(content):
            if line.strip().startswith("graph ["):
                content.insert(i + 1, "  multigraph 1\n")
                break
        
        # 将修改后的内容写回文件
        with open(gml_file, 'w', encoding='utf-8') as file:
            file.writelines(content)

# 计算两点间的距离（基于经纬度）
def haversine_distance(lat1, lon1, lat2, lon2):
    """
    计算两个经纬度之间的球面距离，单位为公里
    参数：每个经纬度以度为单位输入
    """
    # 将角度转换为弧度
    phi1, phi2 = math.radians(lat1), math.radians(lat2)
    delta_phi = math.radians(lat2 - lat1)
    delta_lambda = math.radians(lon2 - lon1)
    
    # 地球半径（单位：公里）
    r = 6371.0
    
    # Haversine 公式
    a = math.sin(delta_phi / 2)**2 + math.cos(phi1) * math.cos(phi2) * math.sin(delta_lambda / 2)**2
    c = 2 * math.asin(math.sqrt(a))
    distance = r * c
    # t(ms) = 0.015*d(km)
    delay = 0.015 * distance
    return delay

# 处理单个GML文件
def process_gml_file(gml_file):
    # 添加multigraph属性
    add_multigraph_attribute(gml_file)
    
    try:
        # 读取GML文件
        G = nx.read_gml(gml_file, label='id')
    except Exception as e:
        print(f"Error reading {gml_file}: {e}")
        return None
    
    # 使用更简单的方法移除重复边
    unique_edges = set()  
    edges_to_remove = []
    for u, v in G.edges():
        if (u, v) in unique_edges or (v, u) in unique_edges:
            edges_to_remove.append((u, v))
        else:
            unique_edges.add((u, v))
    G.remove_edges_from(edges_to_remove)  # 去除重复边
    
    # 为边添加权重（时延）
    for u, v, data in G.edges(data=True):
        # 检查两个节点是否都有经纬度信息
        u_data = G.nodes[u]
        v_data = G.nodes[v]
        
        has_u_coords = ('Latitude' in u_data and 'Longitude' in u_data)
        has_v_coords = ('Latitude' in v_data and 'Longitude' in v_data)
        
        # 如果任一节点缺少经纬度，设置时延为5ms
        if not (has_u_coords and has_v_coords):
            data['delay'] = 5.0
        else:
            # 计算基于地理位置的时延
            lat1, lon1 = float(u_data['Latitude']), float(u_data['Longitude'])
            lat2, lon2 = float(v_data['Latitude']), float(v_data['Longitude'])
            data['delay'] = min(1000.0, max(1.0, haversine_distance(lat1, lon1, lat2, lon2)))
    
    return G

# 主预处理函数
def preprocess_topology_zoo(input_dir, output_dir):
    # 确保输出目录存在
    Path(output_dir).mkdir(parents=True, exist_ok=True)
    
    # 获取所有GML文件
    gml_files = list(Path(input_dir).glob("*.gml"))
    
    # 创建网络列表文件
    network_list_file = os.path.join(output_dir, "networks.txt")
    with open(network_list_file, 'w', encoding='utf-8') as network_list:
        # 写入网络名称列表
        for gml_file in gml_files:
            network_name = gml_file.stem
            print(f"处理网络: {network_name}")
            
            G = process_gml_file(gml_file)
            if G is None:
                continue
            
            # 记录网络名称
            network_list.write(f"{network_name}\n")
            
            # 为每个网络创建单独的边列表文件
            edge_list_file = os.path.join(output_dir, f"{network_name}_edgelist.txt")
            with open(edge_list_file, 'w', encoding='utf-8') as f:
                # 写入节点数和边数信息
                f.write(f"{G.number_of_nodes()} {G.number_of_edges()}\n")
                
                # 写入边信息
                for u, v, data in G.edges(data=True):
                    delay = data.get('delay', 5.0)  # 默认延迟为5ms
                    f.write(f"{u} {v} {delay:.2f}\n")
    
    print(f"预处理完成，网络列表保存至: {network_list_file}")
    print(f"各网络的边列表文件保存在: {output_dir} 目录中")

if __name__ == "__main__":
    input_directory = "dataset"
    output_directory = "cache"
    
    preprocess_topology_zoo(input_directory, output_directory)