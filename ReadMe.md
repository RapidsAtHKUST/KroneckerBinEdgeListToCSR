# Kronecker Bin Edge List Pre-Processing

## Build (Assume in the build directory)

```zsh
cmake project-path
make -j
```

## Usage

* command

```zsh
./kron_converter -f /ssd/zlai/yche-datasets/s25-16.kron.bin -o  /ssd/zlai/yche-datasets/datasets/s25-16.kron.bin 
```

* input-gen: see [RapidsAtHKUST/Graph500KroneckerGraphGenerator](https://github.com/RapidsAtHKUST/Graph500KroneckerGraphGenerator)
* input (edge-list-bin): `/ssd/zlai/yche-datasets/s25-16.kron.bin` 
* output (lijun's csr formats): 

```
ll /ssd/zlai/yche-datasets/datasets/s25-16.kron.bin                                                                                                   ✔  9297  16:49:57
total 4.1G
-rw-r--r-- 1 zlai luo 4.0G Jan  4 16:48 b_adj.bin
-rw-r--r-- 1 zlai luo 129M Jan  4 16:48 b_degree.bin
```
