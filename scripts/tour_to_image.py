import matplotlib.pyplot as plt
import tsplib95 as tsp
import sys

assert(len(sys.argv) == 4)

problem = tsp.load(sys.argv[1])
cities = problem.node_coords
tour = tsp.load(sys.argv[2]).tours[0]
tour_order = tour + [tour[0]]
tour_length = problem.trace_tours([tour])

x = [cities[t][0] for t in tour_order]
y = [cities[t][1] for t in tour_order]

plt.plot(x, y, marker='o')
plt.title('Tour (length = ' + str(tour_length[0]) + ')')
plt.xlabel('x')
plt.ylabel('y')
plt.grid()
plt.savefig(sys.argv[3])
