# IMPORTANT
# to run this code you need to open nix-shell with a few more packages
# if you execute bulid/tsunami_lab the old solution.csv files are just overwrote
# so if you generate less files than within the last run, thex will be appended
# so better delete all the old csv files

# EXAMPLE
# be in tsunami-lab/
# nix-shell
# scons
# ./build/tsunami_lab
# python3 vis.py
# see the animation in solutions/simulation.gif

import pandas as pd
import matplotlib.pyplot as plt
import imageio.v2 as imageio
import glob
import re
import os
import sys

folder = "solutions"

# Dateien numerisch sortieren
files = sorted(
    glob.glob(f"{folder}/solution_*.csv"),
    key=lambda x: int(re.search(r'\d+', x).group())
)

# Existiert mind eine Datei?
if len(files) == 0:
    print("No solution_*.csv files in solutions/ found")
    sys.exit()

images = []

# globales min/max bestimmen
ymin = float("inf")
ymax = float("-inf")

for file in files:
    df = pd.read_csv(file)

    h = df["height"]
    b = df["bathymetry"]
    surface = h + b

    ymin = min(ymin, b.min(), surface.min())
    ymax = max(ymax, b.max(), surface.max())

ymin = -8000
ymax = ymax + 1250

# Plotten
for file in files:
    df = pd.read_csv(file)

    x = df["x"]
    height = df["height"]
    bathymetry = df["bathymetry"]

    surface = height + bathymetry

    plt.figure()
    plt.plot(x, bathymetry, color="saddlebrown", label="bathymetry")
    plt.plot(x, surface, color="blue", label="water surface")
    plt.fill_between(x, bathymetry, surface, alpha=0.3)

    # feste Skala
    plt.ylim(ymin, ymax)

    plt.xlabel("x")
    plt.ylabel("height")
    plt.title(file)

    filename = f"{folder}/temp.png"
    plt.savefig(filename)
    plt.close()

    images.append(imageio.imread(filename))

# GIF erstellen
imageio.mimsave(f"{folder}/simulation.gif", images, fps=2, loop=0) # irgendwie die fps dynamisch an die realen fps anpassen l_dt in main

# remove artefact
if os.path.exists(f"{folder}/temp.png"):
  os.remove(f"{folder}/temp.png")