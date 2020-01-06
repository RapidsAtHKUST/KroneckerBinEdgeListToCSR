# Kronecker Bin Edge List Pre-Processing

## Build 

```zsh
rm -rf ~/build-tmp
mkdir -p ~/build-tmp 
cd ~/build-tmp
cmake -H~/workspace/yche/git-repos/KroneckerBinEdgeListToCSR -B~/build-tmp -DCMAKE_INSTALL_PREFIX=~/yche-bin
make -j 
make install
```

## Usage

* command

```zsh
./kron_converter -f /ssd/zlai/yche-datasets/kron23-16.bin -o /ssd/zlai/yche-datasets/datasets/datasets/kron23-16.bin
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
