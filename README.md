# WitchHutFinder
Find double, triple, quad witch hut on a set seed

# How to run it

Go at https://github.com/hube12/WitchHutFinder/releases and download the latest release.

On windows you can simply double click on the executable, it will display the instructions 
to set it up.

Else you need to use the command line and follow usage() which is:

- `./WitchHutFinder [mcversion] [seed] [searchRange]? [filter]?`

Valid [mcversion] are 1.7, 1.8, 1.9, 1.10, 1.11, 1.12, 1.13, 1.13.2, 1.14.

Valid [searchRange] (optional) is in blocks, default is 150000 which correspond to -150000 to 150000 on both X and Z.

Valid [filter] (optional) is either 2, 3 or 4 for respectively only outputting double, triple or quad witch huts as minimum.


# Examples

./WitchHutFinder 1.12 181201211981019340 100000 2
./WitchHutFinder 1.12 181201211981019340 100000 4
./WitchHutFinder 1.14 181201211981019340 100000 2

# Build
run:

- `cmake .`
- `make WitchHutFinder`