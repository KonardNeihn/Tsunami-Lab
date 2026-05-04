import pandas as pd
import matplotlib.pyplot as plt
import imageio.v2 as imageio
import glob
import re
import os
import sys


def plot_all_station_csvs(folder="solutions/station_data"):
    files = sorted(glob.glob(os.path.join(folder, "*.csv")))

    if not files:
        print(f"No CSV files found in {folder}")
        return

    output_dir = os.path.join(folder, "plots")
    os.makedirs(output_dir, exist_ok=True)

    for csv_file in files:
        df = pd.read_csv(csv_file)

        # Zeit = Zeilenindex
        time = df.index

        plt.figure()

        plt.plot(time, df["height"], label="Height")
        plt.plot(time, df["momentum_x"], label="Momentum X")
        plt.plot(time, df["momentum_y"], label="Momentum Y")

        plt.xlabel("Time step")
        plt.ylabel("Value")
        plt.title(os.path.basename(csv_file))
        plt.legend()
        plt.grid(True)

        plt.tight_layout()

        # Dateiname für Plot
        base_name = os.path.splitext(os.path.basename(csv_file))[0]
        out_path = os.path.join(output_dir, base_name + ".png")

        plt.savefig(out_path)
        plt.close()

        print(f"Saved: {out_path}")


if __name__ == "__main__":
    plot_all_station_csvs()