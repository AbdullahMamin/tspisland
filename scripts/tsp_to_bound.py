import tsplib95 as tsp
import sys
import math

assert(len(sys.argv) == 3)

problem = tsp.load(sys.argv[1])
best_len = int(sys.argv[2])

n_cities = len(list(problem.get_nodes()))

cities = problem.node_coords
x = [cities[t + 1][0] for t in range(n_cities)]
y = [cities[t + 1][1] for t in range(n_cities)]

longest_distance = -1
for i in range(n_cities - 1):
    for j in range(i + 1, n_cities):
        dp = (x[i] - x[j], y[i] - y[j])
        distance = int(0.5 + math.sqrt(dp[0]*dp[0] + dp[1]*dp[1]))
        if distance > longest_distance:
            longest_distance = distance

print(longest_distance/best_len)
