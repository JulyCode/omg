#!/usr/bin/python

import numpy as np
import pandas as pd
from matplotlib import pyplot as plt
from matplotlib.ticker import PercentFormatter
import matplotlib.ticker as ticker

def save_svg(file):
    fig = plt.gcf()
    if latex_export:
        fig.set_size_inches((9, 5), forward=False)
    else:
        fig.set_size_inches((10, 5), forward=False)
    plt.savefig(file, bbox_inches="tight")


latex_export = True

data = pd.read_csv("apps/data/stats.csv", sep=";")
print(data.head())
print(data["tri remesh radius ratio"].count())

radius_ratio = [data["tri radius ratio"], data["tri remesh radius ratio"], data["jig radius ratio"]]
rr_weights = [np.ones(len(d)) / d.count() for d in radius_ratio]

edge_length = [data["tri edge length"], data["tri remesh edge length"], data["jig edge length"]]
el_weights = [np.ones(len(d)) / d.count() for d in edge_length]

valence = [data["tri valence"], data["tri remesh valence"], data["jig valence"]]
#valence = [d[d != 0] for d in valence]
va_weights = [np.ones(len(d)) / d.count() * 100 for d in valence]

labels = ["Triangle", "Triangle remeshed", "Jigsaw"]

plt.style.use("fivethirtyeight")
#plt.style.use("seaborn-deep")
#plt.tight_layout()
if latex_export:
    plt.rcParams["svg.fonttype"] = "none"
    plt.rcParams["axes.unicode_minus"] = False

    percent_formatter = PercentFormatter(1, symbol="\\%")
    log_percent_formatter = ticker.FuncFormatter(lambda y,pos: ('{{:.{:1d}f}}\\%'.format(int(np.maximum(-np.log10(y),0)))).format(y))
else:
    percent_formatter = PercentFormatter(1)
    log_percent_formatter = ticker.FuncFormatter(lambda y,pos: ('{{:.{:1d}f}}%'.format(int(np.maximum(-np.log10(y),0)))).format(y))

# radius ratio
plt.hist(radius_ratio, np.arange(0.0, 1.05, 0.05), weights=rr_weights, label=labels)

if not latex_export:
    plt.title("Quality comparison")
plt.xlabel("Radius ratio", labelpad=10)
plt.ylabel("Triangles", labelpad=10)
plt.legend(loc='upper center')
plt.gca().yaxis.set_major_formatter(percent_formatter)
save_svg("radius_ratio.svg")
plt.show()


# lower half radius ratio
plt.hist(radius_ratio, np.arange(0.0, 0.55, 0.05), weights=rr_weights, label=labels)

if not latex_export:
    plt.title("Quality comparison")
plt.xlabel("Radius ratio", labelpad=10)
plt.ylabel("Triangles", labelpad=10)
plt.legend(loc='upper center')
plt.gca().yaxis.set_major_formatter(percent_formatter)
save_svg("radius_ratio_low.svg")
plt.show()


# relative edge length
plt.hist(edge_length, np.arange(0.05, 2, 0.1), weights=el_weights, label=labels)

if not latex_export:
    plt.title("Quality comparison")
plt.xlabel("Relative edge length", labelpad=10)
plt.ylabel("Edges", labelpad=10)
plt.legend(loc='upper right')
plt.gca().yaxis.set_major_formatter(percent_formatter)
save_svg("edge_length.svg")
plt.show()


# valence deviation
plt.hist(valence, np.arange(-3.5, 4.5, 1), weights=va_weights, label=labels)

if not latex_export:
    plt.title("Quality comparison")
plt.xlabel("Deviation from optimal valence", labelpad=10)
plt.ylabel("Vertices", labelpad=10)
plt.legend(loc='upper right')
plt.yscale('log')
plt.gca().yaxis.set_major_formatter(log_percent_formatter)
save_svg("valence.svg")
plt.show()
