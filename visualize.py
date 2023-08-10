# import networkx as nx
import matplotlib.pyplot as plt
import sys

with open(f"{sys.argv[1]}", "r") as file:
    edges = []
    for line in file:
        edge = line.strip().split(' ')
        u = edge[0]
        v = edge[1]
        edges.append((u, v))

nodes = {}
n = 1

with open("graph.dot", "w") as file:
    file.write("graph D { node [ordering=out]\n")
    for edge in edges:
        if (edge[0] in nodes.keys()):
            file.write(f'{n}[label="{edge[1]}"]\n')
            nodes[edge[1]] = n
            file.write(f'{nodes[edge[0]]} -- {nodes[edge[1]]}\n')
            n += 1
        elif (edge[1] in nodes.keys()):
            file.write(f'{n}[label="{edge[0]}"]\n')
            nodes[edge[0]] = n
            file.write(f'{nodes[edge[0]]} -- {nodes[edge[1]]}\n')
            n += 1
        else:
            file.write(f'{n}[label="{edge[0]}"]\n')
            nodes[edge[0]] = n
            n += 1
            file.write(f'{n}[label="{edge[1]}"]\n')
            nodes[edge[1]] = n
            n += 1
            file.write(f'{nodes[edge[0]]} -- {nodes[edge[1]]}\n')
    file.write("}")

SL_x = []
SL_y = []
SH_x = []
SH_y = []
FL_x = []
FL_y = []
FH_x = []
FH_y = []

Hash_powers = []

with open(f"{sys.argv[2]}", "r") as file:
    while (True):
        line = file.readline()
        if not line:
            break
        Hash_powers.append(float(line.strip().split(' ')[-1]))
        line = file.readline().strip().split(' ')
        
        if (line[2] == '-1'):
            continue
        if (line[0] == 'SL'):
            SL_x.append(int(line[1]))
            SL_y.append(float(line[2]))
        elif (line[0] == 'SH'):
            SH_x.append(int(line[1]))
            SH_y.append(float(line[2]))
        elif (line[0] == 'FL'):
            FL_x.append(int(line[1]))
            FL_y.append(float(line[2]))
        elif (line[0] == 'FH'):
            FH_x.append(int(line[1]))
            FH_y.append(float(line[2]))

    plt.plot(SL_x, SL_y, 'bo', label='SLOW & LOW CPU')
    plt.plot(SH_x, SH_y, 'ro', label='SLOW & HIGH CPU')
    plt.plot(FL_x, FL_y, 'yo', label='FAST & LOW CPU')
    plt.plot(FH_x, FH_y, 'go', label='FAST & HIGH CPU')
    plt.plot(Hash_powers, label='Hash Powers')
    plt.legend(['SLOW & LOW CPU', 'SLOW & HIGH CPU', 'FAST & LOW CPU', 'FAST & HIGH CPU', 'Hash Powers'])
    plt.xlabel('Peer ID')
    plt.title('Simulation Stats')
    plt.savefig(f'{sys.argv[3]}.jpeg')
