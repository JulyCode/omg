#!/usr/bin/python

import pandas as pd

data = pd.read_csv("apps/data/stats.csv", sep=";")

for name in data.columns:
    col = data[name]
    min = str(col.min())
    mean = str(col.mean())
    count = str(col.count())
    print(name + ": " + min + ", " + mean + ", " + count)