# TODO: read file line by line rather than storing the entire thing in memory
# TODO: speed things up a bit if possible

import matplotlib.pyplot as plt
from matplotlib import collections  as mc
import matplotlib.animation as animation
import pylab as pl
import tsplib95 as tsp
import sys
from itertools import groupby

assert(len(sys.argv) == 4)

problem = tsp.load(sys.argv[1])
cities = problem.node_coords

x = [city[0] for city in cities.values()]
y = [city[1] for city in cities.values()]

with open(sys.argv[2], 'r') as heat_file:
    heat_lines = list(map(str.strip,heat_file.readlines()))

edge_heats = []
for k, g in groupby(heat_lines, lambda x: x == '---'):
    if not k:
        edge_heats.append(list(g))

edge_heat_lists = list(map(
    lambda x: [(int(x[i]), int(x[i + 1]), float(x[i + 2])) for i in range(0, len(x), 3)],
    edge_heats
))

# TODO: animation
fig, ax = pl.subplots()
def update(frame):
    print(f"Generating frame {frame}...")
    ax.clear()
    ax.grid(True)
    ax.set_title(f"Population edges (generation: {frame})")
    ax.set_xlabel(f"x")
    ax.set_ylabel(f"y")
    edge_heat_list = edge_heat_lists[frame]
    edges = []
    edge_colors = []
    for edge_heat in edge_heat_list:
        edges.append((cities[edge_heat[0]], cities[edge_heat[1]]))
        edge_colors.append((0, 0, 1, edge_heat[2]))
    edge_collection = mc.LineCollection(edges, colors=edge_colors)
    ax.add_collection(edge_collection)
    ax.scatter(x, y)

ani = animation.FuncAnimation(fig=fig, func=update, frames=len(edge_heat_lists), interval=16)
ani.save(filename=sys.argv[3], writer="ffmpeg")
