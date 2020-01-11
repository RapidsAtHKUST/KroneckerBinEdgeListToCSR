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

# Graph Stats

* command (input is Lijun's CSR binary format two-file folder path: `b_adj.bin`, `b_degree.bin`), you can use the converted files from the `kron_converter`

```zsh
./graph_stats -i /mnt/storage1/yche/datasets/snap_livejournal  
```

* output files under the `/mnt/storage1/yche/datasets/snap_livejournal` folder

```
-rw-r--r-- 1 yche users  118 Jan  9 22:22 /mnt/storage1/yche/datasets/snap_livejournal/basic_stats.txt
-rw-r--r-- 1 yche users 1.5K Jan  9 22:22 /mnt/storage1/yche/datasets/snap_livejournal/kcore.histogram
```

* `basic_stats.txt` as follows

```
|V|: 4,036,538
|E|: 34,681,189
|TC|: 177,820,130
avg-deg: 17.1836
max-deg: 14,815
dodg-max-deg: 524
max-core-val: 360
```

* `kcore.histogram` as follows

```
[38576, 818745, 472810, 334577, 261559, 214592, 182442, 159484, 141088, 129452, 116678, 106590, 97366, 89109, 82902, 76633, 69857, 63254, 58444, 52429, 47611, 42729, 37854, 35465, 29564, 28545, 24336, 19572, 16487, 13990, 12476, 12121, 9531, 9361, 7568, 7332, 5949, 6228, 5637, 4883, 5410, 4248, 4771, 3948, 3930, 2726, 5296, 2173, 2675, 2371, 2469, 1889, 1752, 1405, 1602, 1302, 1614, 2156, 1362, 1320, 1077, 985, 1060, 1061, 1862, 993, 1173, 1212, 1521, 733, 746, 708, 655, 512, 619, 682, 542, 704, 525, 422, 598, 662, 539, 1100, 397, 412, 295, 776, 455, 503, 494, 522, 407, 340, 892, 560, 583, 477, 335, 291, 538, 373, 1726, 130,
22, 169, 125, 15, 10, 30, 46, 144, 15, 10, 50, 290, 16, 13, 12, 8, 24, 9, 7, 155, 10, 176, 154, 14, 58, 12, 15, 232, 5, 4, 11, 17, 25, 12, 18, 15, 17, 5, 11, 12, 157, 48, 169, 215, 24, 18, 10, 20, 11, 46, 48, 188, 4, 9, 8, 25, 44, 9, 9, 219, 2, 5, 6, 22, 2, 11, 195, 4, 15, 7, 7, 8, 12, 9, 16, 373, 193, 45, 210, 2, 13, 7, 0, 113, 0, 4, 2, 2, 2, 4, 1, 1, 1, 3, 9, 8, 5, 3, 2, 11, 4, 2, 4, 4, 2, 49, 8, 9, 9, 6, 8, 11, 8, 7, 8, 42, 4, 9, 12, 10, 7, 3, 8, 8, 24, 209, 1, 8, 2, 4, 6, 4, 4, 3, 3, 1, 11, 4, 43, 6, 5, 275, 3, 14, 6, 0, 47, 356, 7, 4, 4, 6, 1, 0, 1, 0, 4, 14, 10, 4, 3, 1, 9, 10, 7, 6, 4, 2, 8, 8, 0, 9, 344, 1, 0, 2, 0, 0, 3, 1, 1, 0, 0, 0, 0, 0, 7, 15, 0, 1, 15, 2, 306, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 2, 0, 2, 0, 0, 0, 377]
```