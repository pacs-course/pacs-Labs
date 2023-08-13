import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv("results_exp.out", skiprows=5, sep='\t')
df.plot("t", "y", style='.-')
plt.grid("on")
plt.savefig("results_exp.png")