import matplotlib.pyplot as plt
import pandas as pd
import sys

assert(len(sys.argv) == 3 or len(sys.argv) == 4)

fitness_bound = -1
if len(sys.argv) == 4:
    fitness_bound = float(sys.argv[3])

df = pd.read_csv(sys.argv[1])

x = [i for i in range(len(df))]
best_fit = df['max']
avg_fit = df['avg']


plt.title('Population summary')
plt.xlabel('Generation')
plt.plot(x, best_fit, color='green', label='best fitness')
plt.plot(x, avg_fit, color='blue', label='avg fitness')
if fitness_bound != -1:
    plt.plot(x, [fitness_bound for i in range(len(x))], color='red', label='bound')
plt.legend()
plt.savefig(sys.argv[2])
