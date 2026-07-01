import matplotlib.pyplot as plt
import pandas as pd
import sys

assert(len(sys.argv) == 3)

df = pd.read_csv(sys.argv[1])

x = [i for i in range(len(df))]
avg_fit = df['avg']
stddev_fit = df['stddev']
worst_fit = df['min']
best_fit = df['max']
avg_fit_up = [avg + 1.96*stddev for (avg, stddev) in zip(avg_fit, stddev_fit)]
avg_fit_down = [avg - 1.96*stddev for (avg, stddev) in zip(avg_fit, stddev_fit)]

plt.title('Population summary')
plt.xlabel('Generation')
plt.plot(x, avg_fit_down, color='blue', linestyle='--')
plt.plot(x, avg_fit, color='blue', label='avg fitness (95% conf)')
plt.plot(x, avg_fit_up, color='blue', linestyle='--')
plt.plot(x, best_fit, color='green', label='best fitness')
plt.plot(x, worst_fit, color='red', label='worst fitness')
plt.legend()
plt.savefig(sys.argv[2])
