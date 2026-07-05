import matplotlib.pyplot as plt
import sys
import math

assert(len(sys.argv) == 3)

with open(sys.argv[1], 'r') as file:
    profiles = file.read().strip().split('---')

profiles = map(lambda x: map(lambda y: float(y.split(',')[-1]), x.split()), profiles)
entropy = []
for i, profile in enumerate(profiles):
    e = 0.0
    for p in profile:
        e -= p*math.log(p)
    entropy.append(e)
entropy = entropy[:-2]
x = [i for i in range(len(entropy))]

plt.title('Population edge entropy')
plt.xlabel('Generation')
plt.plot(x, entropy, color='blue')
plt.savefig(sys.argv[2])
