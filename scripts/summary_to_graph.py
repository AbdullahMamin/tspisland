import matplotlib.pyplot as plt
import pandas as pd
import sys

assert(len(sys.argv) == 3)

df = pd.read_csv(sys.argv[1])

x = [i for i in range(len(df))]
best_fit = df['max']
avg_fit = df['avg']

plt.title('Population summary')
plt.xlabel('Generation')
plt.plot(x, best_fit, color='green', label='best fitness')
plt.plot(x, avg_fit, color='blue', label='avg fitness (95% conf)')
plt.legend()
plt.savefig(sys.argv[2])
