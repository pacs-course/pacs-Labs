#!/usr/bin/env python3
"""Plot the binary output produced by the MPI heat-equation examples."""

from argparse import ArgumentParser
from pathlib import Path

import matplotlib

matplotlib.use("Agg")

import matplotlib.pyplot as plt
import numpy as np


def read_output(path):
    """Read dimensions, final time, and field values from a solver output."""
    with path.open("rb") as file:
        grid = np.fromfile(file, dtype=np.uint64, count=2)
        if grid.size != 2:
            raise ValueError(f"{path}: missing grid-size header")

        time = np.fromfile(file, dtype=np.float64, count=1)
        if time.size != 1:
            raise ValueError(f"{path}: missing final-time header")

        nx, ny = map(int, grid)
        values = np.fromfile(file, dtype=np.float64)

    expected = nx * ny
    if values.size != expected:
        raise ValueError(f"{path}: expected {expected} values, found {values.size}")

    return nx, ny, float(time[0]), values.reshape((nx, ny))


def visualize(name="output", image=None, style="dark_background"):
    """Render a solver output file to a PNG image."""
    path = Path(name)
    image_path = Path(image) if image is not None else path.with_name(path.name + ".png")

    nx, ny, time, values = read_output(path)
    print(f"Plotting grid {nx}x{ny}, t = {time}")

    plt.style.use(style)
    fig, ax = plt.subplots()
    mesh = ax.pcolormesh(values, cmap=plt.cm.jet, vmin=0, vmax=values.max())
    ax.set_title(f"Temperature at t = {time:.3f} [s]")
    ax.set_xlabel("x")
    ax.set_ylabel("y")
    fig.colorbar(mesh, ax=ax)
    fig.savefig(image_path, transparent=True, bbox_inches="tight", dpi=300)
    plt.close(fig)

    print(f"Wrote {image_path}")


def main():
    parser = ArgumentParser(description=__doc__)
    parser.add_argument("output", nargs="?", default="output",
                        help="binary solver output to plot (default: output)")
    parser.add_argument("-o", "--image",
                        help="PNG file to write (default: <output>.png)")
    parser.add_argument("--style", default="dark_background",
                        help="matplotlib style to use (default: dark_background)")
    args = parser.parse_args()

    visualize(args.output, image=args.image, style=args.style)


if __name__ == "__main__":
    main()
