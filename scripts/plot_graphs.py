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

plt.style.use("fivethirtyeight")
#plt.tight_layout()

if latex_export:
    plt.rcParams["svg.fonttype"] = "none"
    plt.rcParams["axes.unicode_minus"] = False
    #plt.rcParams['axes.facecolor'] = 'white'
    #plt.rcParams['figure.facecolor'] = 'white'
    plt.rcParams['font.size'] = "17"

    percent_formatter = PercentFormatter(1, symbol="\\%")
    log_percent_formatter = ticker.FuncFormatter(lambda y,pos: ('{{:.{:1d}f}}\\%'.format(int(np.maximum(-np.log10(y),0)))).format(y))
else:
    percent_formatter = PercentFormatter(1)
    log_percent_formatter = ticker.FuncFormatter(lambda y,pos: ('{{:.{:1d}f}}%'.format(int(np.maximum(-np.log10(y),0)))).format(y))


# radius ratio jig tri
radius_ratio = [data["tri radius ratio"], data["jig radius ratio"]]
weights = [np.ones(len(d)) / d.count() for d in radius_ratio]
labels = ["Triangle", "Jigsaw"]

plt.hist(radius_ratio, np.arange(0.0, 1.05, 0.05), weights=weights, label=labels)

if not latex_export:
    plt.title("Quality comparison")
plt.xlabel("Radius ratio", labelpad=10)
plt.ylabel("Triangles", labelpad=10)
plt.legend(loc='upper center')
plt.gca().yaxis.set_major_formatter(percent_formatter)
plt.gca().yaxis.grid(color='#cccccc')
plt.gca().xaxis.grid(color='#cccccc')
plt.gca().set_axisbelow(True)
save_svg("radius_ratio_tri_jig.svg")
plt.show()

# radius ratio tri tri_remesh jig jig_remesh
radius_ratio = [data["tri radius ratio"], data["jig radius ratio"], data["tri remesh radius ratio"], data["jig remesh radius ratio"]]
weights = [np.ones(len(d)) / d.count() for d in radius_ratio]
labels = ["Triangle", "Jigsaw", "Triangle remeshed", "Jigsaw remeshed"]

plt.hist(radius_ratio, np.arange(0.5, 1.05, 0.05), weights=weights, label=labels)

if not latex_export:
    plt.title("Quality comparison")
plt.xlabel("Radius ratio", labelpad=10)
plt.ylabel("Triangles", labelpad=10)
plt.legend(loc='upper center')
plt.gca().yaxis.set_major_formatter(percent_formatter)
plt.gca().yaxis.grid(color='#cccccc')
plt.gca().xaxis.grid(color='#cccccc')
plt.gca().set_axisbelow(True)
save_svg("radius_ratio_both_remesh_upper.svg")
plt.show()

# radius ratio tri tri_remesh jig jig_remesh lower
plt.hist(radius_ratio, np.arange(0.0, 0.55, 0.05), label=labels)

if latex_export:
    small_percent_formatter = PercentFormatter(1, decimals=3, symbol="\\%")
else:
    small_percent_formatter = PercentFormatter(1, decimals=3)

if not latex_export:
    plt.title("Quality comparison")
plt.xlabel("Radius ratio", labelpad=10)
plt.ylabel("Triangles", labelpad=10)
plt.legend(loc='upper center')
#plt.gca().yaxis.set_major_formatter(small_percent_formatter)
plt.gca().yaxis.grid(color='#cccccc')
plt.gca().xaxis.grid(color='#cccccc')
plt.gca().set_axisbelow(True)
save_svg("radius_ratio_both_remesh_lower.svg")
plt.show()

# shape regularity tri tri_remesh jig jig_remesh
shape_reg = [data["tri shape regularity"], data["jig shape regularity"], data["tri remesh shape regularity"], data["jig remesh shape regularity"]]
weights = [np.ones(len(d)) / d.count() for d in shape_reg]
labels = ["Triangle", "Jigsaw", "Triangle remeshed", "Jigsaw remeshed"]

plt.hist(shape_reg, np.arange(0.5, 1.05, 0.05), weights=weights, label=labels)

if not latex_export:
    plt.title("Quality comparison")
plt.xlabel("Shape regularity", labelpad=10)
plt.ylabel("Triangles", labelpad=10)
plt.legend(loc='upper center')
plt.gca().yaxis.set_major_formatter(percent_formatter)
plt.gca().yaxis.grid(color='#cccccc')
plt.gca().xaxis.grid(color='#cccccc')
plt.gca().set_axisbelow(True)
save_svg("shape_regularity_both_remesh_upper.svg")
plt.show()

# shape regularity tri tri_remesh jig jig_remesh lower
plt.hist(shape_reg, np.arange(-0.2, 0.55, 0.05), label=labels)

if not latex_export:
    plt.title("Quality comparison")
plt.xlabel("Shape regularity", labelpad=10)
plt.ylabel("Triangles", labelpad=10)
plt.legend(loc='upper center')
#plt.gca().yaxis.set_major_formatter(percent_formatter)
plt.gca().yaxis.grid(color='#cccccc')
plt.gca().xaxis.grid(color='#cccccc')
plt.gca().set_axisbelow(True)
save_svg("shape_regularity_both_remesh_lower.svg")
plt.show()


# shape regularity tri tri_limited tri_marche
shape_reg = [data["tri shape regularity"], data["tri limited shape regularity"], data["tri marche shape regularity"]]
weights = [np.ones(len(d)) / d.count() for d in shape_reg]
labels = ["Triangle", "Fast limiting", "Marche limiting"]

plt.hist(shape_reg, np.arange(0.5, 1.05, 0.05), weights=weights, label=labels)

if not latex_export:
    plt.title("Quality comparison")
plt.xlabel("Shape regularity", labelpad=10)
plt.ylabel("Triangles", labelpad=10)
plt.legend(loc='upper center')
plt.gca().yaxis.set_major_formatter(percent_formatter)
plt.gca().yaxis.grid(color='#cccccc')
plt.gca().xaxis.grid(color='#cccccc')
plt.gca().set_axisbelow(True)
save_svg("shape_regularity_tri_limited_upper.svg")
plt.show()

# shape regularity tri tri_limited tri_marche lower
plt.hist(shape_reg, np.arange(0.0, 0.55, 0.05), label=labels)

if not latex_export:
    plt.title("Quality comparison")
plt.xlabel("Shape regularity", labelpad=10)
plt.ylabel("Triangles", labelpad=10)
plt.legend(loc='upper center')
#plt.gca().yaxis.set_major_formatter(percent_formatter)
plt.gca().yaxis.grid(color='#cccccc')
plt.gca().xaxis.grid(color='#cccccc')
plt.gca().set_axisbelow(True)
save_svg("shape_regularity_tri_limited_lower.svg")
plt.show()

# shape regularity tri_remesh tri_remesh_limited tri_remesh_marche
shape_reg = [data["tri remesh shape regularity"], data["tri remesh limited shape regularity"], data["tri remesh marche shape regularity"]]
weights = [np.ones(len(d)) / d.count() for d in shape_reg]
labels = ["Triangle remeshed", "Fast limiting", "Marche limiting"]

plt.hist(shape_reg, np.arange(0.5, 1.05, 0.05), weights=weights, label=labels)

if not latex_export:
    plt.title("Quality comparison")
plt.xlabel("Shape regularity", labelpad=10)
plt.ylabel("Triangles", labelpad=10)
plt.legend(loc='upper center')
plt.gca().yaxis.set_major_formatter(percent_formatter)
plt.gca().yaxis.grid(color='#cccccc')
plt.gca().xaxis.grid(color='#cccccc')
plt.gca().set_axisbelow(True)
save_svg("shape_regularity_tri_remesh_limited_upper.svg")
plt.show()

# shape regularity tri_remesh tri_remesh_limited tri_remesh_marche lower
plt.hist(shape_reg, np.arange(-0.2, 0.55, 0.05), label=labels)

if not latex_export:
    plt.title("Quality comparison")
plt.xlabel("Shape regularity", labelpad=10)
plt.ylabel("Triangles", labelpad=10)
plt.legend(loc='upper center')
#plt.gca().yaxis.set_major_formatter(percent_formatter)
plt.gca().yaxis.grid(color='#cccccc')
plt.gca().xaxis.grid(color='#cccccc')
plt.gca().set_axisbelow(True)
save_svg("shape_regularity_tri_remesh_limited_lower.svg")
plt.show()

# shape regularity jig jig_limited jig_marche
shape_reg = [data["jig shape regularity"], data["jig limited shape regularity"], data["jig marche shape regularity"]]
weights = [np.ones(len(d)) / d.count() for d in shape_reg]
labels = ["Jigsaw", "Fast limiting", "Marche limiting"]

plt.hist(shape_reg, np.arange(0.5, 1.05, 0.05), weights=weights, label=labels)

if not latex_export:
    plt.title("Quality comparison")
plt.xlabel("Shape regularity", labelpad=10)
plt.ylabel("Triangles", labelpad=10)
plt.legend(loc='upper center')
plt.gca().yaxis.set_major_formatter(percent_formatter)
plt.gca().yaxis.grid(color='#cccccc')
plt.gca().xaxis.grid(color='#cccccc')
plt.gca().set_axisbelow(True)
save_svg("shape_regularity_jig_limited.svg")
plt.show()


# radius ratio
radius_ratio = [data["tri radius ratio"], data["tri remesh radius ratio"], data["jig radius ratio"]]
rr_weights = [np.ones(len(d)) / d.count() for d in radius_ratio]
labels = ["Triangle", "Triangle remeshed", "Jigsaw"]

plt.hist(radius_ratio, np.arange(0.6, 1.05, 0.05), weights=rr_weights, label=labels)

if not latex_export:
    plt.title("Quality comparison")
plt.xlabel("Radius ratio", labelpad=10)
plt.ylabel("Triangles", labelpad=10)
plt.legend(loc='upper center')
plt.gca().yaxis.set_major_formatter(percent_formatter)
plt.gca().yaxis.grid(color='#cccccc')
plt.gca().xaxis.grid(color='#cccccc')
plt.gca().set_axisbelow(True)
save_svg("radius_ratio.svg")
plt.show()

# lower half radius ratio
plt.hist(radius_ratio, np.arange(0.0, 0.55, 0.05), weights=rr_weights, label=labels)

if latex_export:
    small_percent_formatter = PercentFormatter(1, decimals=3, symbol="\\%")
else:
    small_percent_formatter = PercentFormatter(1, decimals=3)

if not latex_export:
    plt.title("Quality comparison")
plt.xlabel("Radius ratio", labelpad=10)
plt.ylabel("Triangles", labelpad=10)
plt.legend(loc='upper center')
plt.gca().yaxis.set_major_formatter(small_percent_formatter)
plt.gca().yaxis.grid(color='#cccccc')
plt.gca().xaxis.grid(color='#cccccc')
plt.gca().set_axisbelow(True)
save_svg("radius_ratio_low.svg")
plt.show()


# relative edge length
edge_length = [data["tri edge length"], data["jig edge length"], data["tri remesh edge length"]]
el_weights = [np.ones(len(d)) / d.count() for d in edge_length]
labels = ["Triangle", "Jigsaw", "Triangle remeshed"]

plt.hist(edge_length, np.arange(0.05, 2, 0.1), weights=el_weights, label=labels)

if not latex_export:
    plt.title("Quality comparison")
plt.xlabel("Relative edge length", labelpad=10)
plt.ylabel("Edges", labelpad=10)
plt.legend(loc='upper right')
plt.gca().yaxis.set_major_formatter(percent_formatter)
plt.gca().yaxis.grid(color='#cccccc')
plt.gca().xaxis.grid(color='#cccccc')
plt.gca().set_axisbelow(True)
save_svg("edge_length.svg")
plt.show()


edge_length = [data["tri limited edge length"], data["jig limited edge length"], data["tri remesh limited edge length"]]
el_weights = [np.ones(len(d)) / d.count() for d in edge_length]

plt.hist(edge_length, np.arange(0.05, 2, 0.1), weights=el_weights, label=labels)

if not latex_export:
    plt.title("Quality comparison")
plt.xlabel("Relative edge length", labelpad=10)
plt.ylabel("Edges", labelpad=10)
plt.legend(loc='upper right')
plt.gca().yaxis.set_major_formatter(percent_formatter)
plt.gca().yaxis.grid(color='#cccccc')
plt.gca().xaxis.grid(color='#cccccc')
plt.gca().set_axisbelow(True)
save_svg("edge_length_limited.svg")
plt.show()


# valence deviation
valence = [data["tri valence"], data["tri remesh valence"], data["jig valence"]]
#valence = [d[d != 0] for d in valence]
va_weights = [np.ones(len(d)) / d.count() * 100 for d in valence]
labels = ["Triangle", "Jigsaw", "Triangle remeshed"]

plt.hist(valence, np.arange(-3.5, 4.5, 1), weights=va_weights, label=labels)

if not latex_export:
    plt.title("Quality comparison")
plt.xlabel("Deviation from optimal valence", labelpad=10)
plt.ylabel("Vertices", labelpad=10)
plt.legend(loc='upper right')
plt.yscale('log')
plt.gca().yaxis.set_major_formatter(log_percent_formatter)
plt.gca().yaxis.grid(color='#cccccc')
plt.gca().xaxis.grid(color='#cccccc')
plt.gca().set_axisbelow(True)
save_svg("valence.svg")
plt.show()


valence = [data["tri limited valence"], data["tri remesh limited valence"], data["jig limited valence"]]
#valence = [d[d != 0] for d in valence]
va_weights = [np.ones(len(d)) / d.count() * 100 for d in valence]
plt.hist(valence, np.arange(-3.5, 4.5, 1), weights=va_weights, label=labels)

if not latex_export:
    plt.title("Quality comparison")
plt.xlabel("Deviation from optimal valence", labelpad=10)
plt.ylabel("Vertices", labelpad=10)
plt.legend(loc='upper right')
plt.yscale('log')
plt.gca().yaxis.set_major_formatter(log_percent_formatter)
plt.gca().yaxis.grid(color='#cccccc')
plt.gca().xaxis.grid(color='#cccccc')
plt.gca().set_axisbelow(True)
save_svg("valence_limited.svg")
plt.show()
