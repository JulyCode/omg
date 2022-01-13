import numpy as np
import pandas as pd
from matplotlib import pyplot as plt
from matplotlib.ticker import PercentFormatter
import matplotlib.ticker as ticker

latex_export = True

def save_svg(file):
    fig = plt.gcf()
    if latex_export:
        fig.set_size_inches((9, 5), forward=False)
    else:
        fig.set_size_inches((10, 5), forward=False)
    plt.savefig(file, bbox_inches="tight")

data = pd.read_csv("apps/data/limit_stats.csv", sep=";")

limits = [0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0]
shape_min = np.zeros(len(data.columns))
shape_avg = np.zeros(len(data.columns))
count = np.zeros(len(data.columns))

for i in range(len(data.columns)):
    col = data[data.columns[i]]
    shape_min[i] = col.min()
    shape_avg[i] = col.mean()
    count[i] = col.count()

base_count = count[len(data.columns) - 1]
for i in range(len(data.columns)):
    count[i] = count[i] / base_count - 1

plt.style.use("fivethirtyeight")

plt.rcParams["svg.fonttype"] = "none"
plt.rcParams["axes.unicode_minus"] = False
#plt.rcParams['axes.facecolor'] = 'white'
#plt.rcParams['figure.facecolor'] = 'white'
plt.rcParams['font.size'] = "16"

lbs = [i for i in limits]
x = np.arange(len(lbs))
width = 0.35
fig, ax = plt.subplots()
rects1 = ax.bar(x - width/2, shape_min, width, label='Worst')
rects2 = ax.bar(x + width/2, shape_avg, width, label='Average')
#rects3 = ax.bar(x + width/2, count, width, label='# Triangles')
ax.set_ylabel('Shape regularity', labelpad=10)
ax.set_xlabel('Gradient limit', labelpad=10)
ax.set_xticks(x)
ax.set_xticklabels(lbs)
ax.legend(loc='lower left')

plt.gca().yaxis.grid(color='#cccccc')
plt.gca().xaxis.grid(color='#cccccc')

#fig.tight_layout()

save_svg("gradient_limits.svg")
plt.show()


