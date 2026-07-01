import matplotlib.pyplot as plt
import pandas as pd
import sys

assert(len(sys.argv) == 3)

df = pd.read_csv(sys.argv[1])

x = [i for i in range(len(df))]
entropy = df['entropy']

plt.title('Population edge entropy')
plt.xlabel('Generation')
plt.plot(x, entropy, color='blue')
plt.savefig(sys.argv[2])
