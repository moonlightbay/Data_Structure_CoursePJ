#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <math.h>

#define MAX_NODES 1000
#define MAX_EDGES 10000
#define MAX_NAME_LENGTH 100

// 边结构
typedef struct {
    int to;
    double delay;
} Edge;

// 图结构
typedef struct {
    Edge* edges[MAX_NODES];
    int edge_count[MAX_NODES];
    int edge_capacity[MAX_NODES];
    Edge* original_edges[MAX_NODES];
    int original_edge_count[MAX_NODES];
    int original_edge_capacity[MAX_NODES];
    int node_status[MAX_NODES];  // 1表示启用，0表示停用
    int node_count;
} Graph;

// 优先队列节点
typedef struct {
    int id;
    double dist;
} QueueNode;

// 优先队列
typedef struct {
    QueueNode* nodes;
    int size;
    int capacity;
} PriorityQueue;

// 初始化优先队列
void init_queue(PriorityQueue* queue) {
    queue->capacity = MAX_NODES;
    queue->size = 0;
    queue->nodes = (QueueNode*)malloc(sizeof(QueueNode) * queue->capacity);
}

// 释放优先队列
void free_queue(PriorityQueue* queue) {
    free(queue->nodes);
}

// 交换两个优先队列节点
void swap_nodes(QueueNode* a, QueueNode* b) {
    QueueNode temp = *a;
    *a = *b;
    *b = temp;
}

// 上浮操作
void swim(PriorityQueue* queue, int k) {
    while (k > 0) {
        int parent = (k - 1) / 2;
        if (queue->nodes[k].dist < queue->nodes[parent].dist) {
            swap_nodes(&queue->nodes[k], &queue->nodes[parent]);
            k = parent;
        } else {
            break;
        }
    }
}

// 下沉操作
void sink(PriorityQueue* queue, int k) {
    while (2 * k + 1 < queue->size) {
        int j = 2 * k + 1;
        if (j + 1 < queue->size && queue->nodes[j+1].dist < queue->nodes[j].dist) {
            j++;
        }
        if (queue->nodes[k].dist <= queue->nodes[j].dist) {
            break;
        }
        swap_nodes(&queue->nodes[k], &queue->nodes[j]);
        k = j;
    }
}

// 入队
void enqueue(PriorityQueue* queue, int id, double dist) {
    queue->nodes[queue->size].id = id;
    queue->nodes[queue->size].dist = dist;
    swim(queue, queue->size);
    queue->size++;
}

// 出队
QueueNode dequeue(PriorityQueue* queue) {
    QueueNode min = queue->nodes[0];
    queue->nodes[0] = queue->nodes[--queue->size];
    sink(queue, 0);
    return min;
}

// 初始化图
void init_graph(Graph* graph) {
    for (int i = 0; i < MAX_NODES; i++) {
        graph->edge_count[i] = 0;
        graph->edge_capacity[i] = 10; // 初始容量
        graph->edges[i] = (Edge*)malloc(sizeof(Edge) * graph->edge_capacity[i]);
        
        graph->original_edge_count[i] = 0;
        graph->original_edge_capacity[i] = 10; // 初始容量
        graph->original_edges[i] = (Edge*)malloc(sizeof(Edge) * graph->original_edge_capacity[i]);
        
        graph->node_status[i] = 1; // 默认启用
    }
    graph->node_count = 0;
}

// 释放图
void free_graph(Graph* graph) {
    for (int i = 0; i < MAX_NODES; i++) {
        free(graph->edges[i]);
        free(graph->original_edges[i]);
    }
}

// 添加边
void add_edge(Graph* graph, int from, int to, double delay) {
    // 扩容检查
    if (graph->edge_count[from] == graph->edge_capacity[from]) {
        graph->edge_capacity[from] *= 2;
        graph->edges[from] = (Edge*)realloc(graph->edges[from], sizeof(Edge) * graph->edge_capacity[from]);
    }
    
    // 添加边
    graph->edges[from][graph->edge_count[from]].to = to;
    graph->edges[from][graph->edge_count[from]].delay = delay;
    graph->edge_count[from]++;
    
    // 原始图也添加边
    if (graph->original_edge_count[from] == graph->original_edge_capacity[from]) {
        graph->original_edge_capacity[from] *= 2;
        graph->original_edges[from] = (Edge*)realloc(graph->original_edges[from], sizeof(Edge) * graph->original_edge_capacity[from]);
    }
    
    graph->original_edges[from][graph->original_edge_count[from]].to = to;
    graph->original_edges[from][graph->original_edge_count[from]].delay = delay;
    graph->original_edge_count[from]++;
}

// 从文件加载网络拓扑
int load_topology(Graph* graph, const char* network_name, int start_node) {
    char filename[MAX_NAME_LENGTH + 20];
    sprintf(filename, "cache/%s_edgelist.txt", network_name);

    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("无法打开文件: %s\n", filename);
        return 0;
    }

    int node_count, edge_count;
    fscanf(file, "%d %d", &node_count, &edge_count);
    
    graph->node_count = node_count;
    
    for (int i = 0; i < edge_count; i++) {
        int from, to;
        double delay;
        fscanf(file, "%d %d %lf", &from, &to, &delay);
        
        // 无向图，添加双向边
        add_edge(graph, from, to, delay);
        add_edge(graph, to, from, delay);
    }
    
    fclose(file);
    return 1;
}

// 停用节点
void disable_nodes(Graph* graph, int* nodes, int count, int start_node) {
    for (int i = 0; i < count; i++) {
        if (nodes[i] != start_node) { // 不允许停用起始节点
            graph->node_status[nodes[i]] = 0;
        }
    }
}

// 启用节点
void enable_nodes(Graph* graph, int* nodes, int count) {
    for (int i = 0; i < count; i++) {
        graph->node_status[nodes[i]] = 1;
    }
}

// 链路时延波动
void fluctuate_links(Graph* graph, int* from_nodes, int* to_nodes, double* deltas, int count) {
    for (int i = 0; i < count; i++) {
        int from = from_nodes[i];
        int to = to_nodes[i];
        double delta = deltas[i];
        
        // 更新两个方向的边
        for (int j = 0; j < graph->edge_count[from]; j++) {
            if (graph->edges[from][j].to == to) {
                double new_delay = graph->edges[from][j].delay + delta;
                
                // 确保时延在1-1000ms范围内
                if (new_delay < 1.0) new_delay = 1.0;
                if (new_delay > 1000.0) new_delay = 1000.0;
                
                graph->edges[from][j].delay = new_delay;
                break;
            }
        }
        
        for (int j = 0; j < graph->edge_count[to]; j++) {
            if (graph->edges[to][j].to == from) {
                double new_delay = graph->edges[to][j].delay + delta;
                
                // 确保时延在1-1000ms范围内
                if (new_delay < 1.0) new_delay = 1.0;
                if (new_delay > 1000.0) new_delay = 1000.0;
                
                graph->edges[to][j].delay = new_delay;
                break;
            }
        }
    }
}

// Dijkstra算法
void dijkstra(Graph* graph, int start, double* dist) {
    // 初始化距离数组
    for (int i = 0; i < graph->node_count; i++) {
        dist[i] = DBL_MAX;
    }
    dist[start] = 0;
    
    // 初始化优先队列
    PriorityQueue queue;
    init_queue(&queue);
    enqueue(&queue, start, 0);
    
    while (queue.size > 0) {
        QueueNode current = dequeue(&queue);
        int u = current.id;
        
        // 跳过已停用节点
        if (graph->node_status[u] == 0) continue;
        
        // 如果距离比已知的大，跳过
        if (current.dist > dist[u]) continue;
        
        // 遍历所有邻居
        for (int i = 0; i < graph->edge_count[u]; i++) {
            Edge e = graph->edges[u][i];
            int v = e.to;
            
            // 跳过已停用节点
            if (graph->node_status[v] == 0) continue;
            
            double new_dist = dist[u] + e.delay;
            
            // 更新距离
            if (new_dist < dist[v]) {
                dist[v] = new_dist;
                enqueue(&queue, v, new_dist);
            }
        }
    }
    
    free_queue(&queue);
}

// 更新并输出最短路径
void update_and_print(Graph* graph, int start_node) {
    double dist[MAX_NODES];
    dijkstra(graph, start_node, dist);
    
    int first = 1;
    for (int i = 0; i < graph->node_count; i++) {
        if (i == start_node) continue;
        
        if (!first) printf(" ");
        first = 0;
        
        if (dist[i] == DBL_MAX) {
            printf("-1.0");
        } else {
            printf("%.2f", dist[i]);
        }
    }
    printf("\n");
}

int main() {
    char network_name[MAX_NAME_LENGTH];
    int start_node;
    int event_count;
    char line[MAX_NAME_LENGTH * 2];

    // 处理每个测试用例
    while (fgets(line, sizeof(line), stdin) != NULL) {
        // 去除行尾换行符
        line[strcspn(line, "\n")] = 0;
        
        // 如果行为空，继续读取
        if (strlen(line) == 0) continue;
        
        // 复制网络名称
        strcpy(network_name, line);
        
        // 读取起始节点和事件数量
        scanf("%d %d", &start_node, &event_count);
        getchar(); // 吸收换行符
        
        // 初始化网络路由器
        Graph graph;
        init_graph(&graph);
        
        if (!load_topology(&graph, network_name, start_node)) {
            return 1;
        }
        
        // 输出初始状态
        update_and_print(&graph, start_node);
        
        // 处理事件
        for (int i = 0; i < event_count; i++) {
            char event_type[10];
            scanf("%s", event_type);
            
            if (strcmp(event_type, "down") == 0) {  // 停用节点
                int count;
                scanf("%d", &count);
                
                int* nodes = (int*)malloc(sizeof(int) * count);
                for (int j = 0; j < count; j++) {
                    scanf("%d", &nodes[j]);
                }
                
                disable_nodes(&graph, nodes, count, start_node);
                free(nodes);
            }
            else if (strcmp(event_type, "up") == 0) {  // 启用节点
                int count;
                scanf("%d", &count);
                
                int* nodes = (int*)malloc(sizeof(int) * count);
                for (int j = 0; j < count; j++) {
                    scanf("%d", &nodes[j]);
                }
                
                enable_nodes(&graph, nodes, count);
                free(nodes);
            }
            else if (strcmp(event_type, "fluc") == 0) { // 链路时延波动
                int count;
                scanf("%d", &count);
                
                int* from_nodes = (int*)malloc(sizeof(int) * count);
                int* to_nodes = (int*)malloc(sizeof(int) * count);
                double* deltas = (double*)malloc(sizeof(double) * count);
                
                for (int j = 0; j < count; j++) {
                    scanf("%d %d %lf", &from_nodes[j], &to_nodes[j], &deltas[j]);
                }
                
                fluctuate_links(&graph, from_nodes, to_nodes, deltas, count);
                
                free(from_nodes);
                free(to_nodes);
                free(deltas);
            }
            else if (strcmp(event_type, "update") == 0) { // 更新最短路径
                update_and_print(&graph, start_node);
            }
        }
        
        free_graph(&graph);
    }
    
    return 0;
}