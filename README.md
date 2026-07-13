## tspisland

A distributed genetic algorithm using the island-migration model for solving the Traveling Salesman Problem (TSP).

## About

This is the repository that has all the code I developed for my MSc thesis.

Island topologies can be set in a toml format; examples are provided in the "examples" directory.

# Building

The code is intended to work on a linux system. You will need a few requirements to build the code:
* make
* gcc
* openmpi and its headers
* python3 (packages: pandas, matplotlib, TSPLib95)

Installing these packages on debian can be done with the following commands:
```
sudo apt install make gcc openmpi-bin libopenmpi-dev python3 python3-pip python3-venv
```

I recommend that you create a python virtual environment with the required packages installed in order to avoid any issues, especially on debian-based distributions.

Once all packages are installed and the virtual environment is set up, run the following command to build:

```make```

You can also do the following command to run a test, change experiments.sh for different tests:

```make experiments```

All outputs, including the binary, go to the "out" directory.

## Thesis paper

TODO: Will add a link to thesis paper when finished.

## Contact

Abdullah M. Abdullah (100068011@ku.ac.ae)

## LICENSE

tspisland

(C) 2026  Abdullah M. Abdullah

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.

## ATTRIBUTIONS & ACKNOWLEDGMENTS

[TSPLIB data](https://github.com/mastqe/tsplib)
[TSPArt data](https://www.math.uwaterloo.ca/tsp/data/art/)
[TOML parser](https://github.com/cktan/tomlc17)
[32-bit hash](https://github.com/skeeto/hash-prospector/issues/19)
