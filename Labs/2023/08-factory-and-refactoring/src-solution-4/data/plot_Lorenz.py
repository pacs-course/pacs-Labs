import pandas as pd
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import axes3d

df = pd.read_csv("results_Lorenz.out", skiprows=5, sep='\t')
plt.figure(figsize=(12, 5))
for i in range(3):
  label = f"y[{i}]"
  plt.plot(df.t, df[label], 'o-', markersize=0.5, label=label)
plt.grid("on")
plt.xlabel("t")
plt.legend()
plt.savefig("results_Lorenz_time.png")
plt.close()

ax = plt.figure(figsize=(10, 10)).add_subplot(projection='3d')
ax.plot(*[df[f"y[{i}]"] for i in range(3)])
plt.savefig("results_Lorenz_space.png")
