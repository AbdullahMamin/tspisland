import matplotlib.pyplot as plt
import pandas as pd
import sys

assert(len(sys.argv) == 4)

df = pd.read_csv(sys.argv[1])
avg_fit = df['avg_fitness']
stddev_fit = df['stddev_fitness']
worst_fit = df['worst_fitness']
best_fit = df['best_fitness']
edge_entropy = df['edge_entropy']
avg_fit_up = avg_fit + 1.96*stddev_fit
avg_fit_down = avg_fit - 1.96*stddev_fit

x = [i for i in range(len(avg_fit))]

plt.title('Population summary')
plt.xlabel('Generation')
plt.plot(x, avg_fit_down, color='blue', linestyle='--')
plt.plot(x, avg_fit, color='blue', label='avg fitness (95% conf)')
plt.plot(x, avg_fit_up, color='blue', linestyle='--')
plt.plot(x, best_fit, color='green', label='best fitness')
plt.plot(x, worst_fit, color='red', label='worst fitness')
plt.legend()
plt.savefig(sys.argv[2])

plt.cla()
plt.clf()
plt.title('Population edge entropy')
plt.xlabel('Generation')
plt.plot(x, edge_entropy)
plt.savefig(sys.argv[3])
