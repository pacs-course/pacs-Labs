import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv("results_VdP.out", skiprows=5, sep='\t')
for i in range(2):
  label = f"y[{i}]"
  plt.plot(df.t, df[label], 'o-', markersize=2, label=label)
plt.grid("on")
plt.xlabel("t")
plt.legend()
plt.savefig("results_VdP.png")
