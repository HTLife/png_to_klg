# png to klg format convertor for ElasticFusion

Convert [TUM RGB-D png dataset](http://vision.in.tum.de/data/datasets/rgbd-dataset/download#freiburg1_desk) to .klg format for [Kintinuous](https://github.com/mp3guy/Kintinuous) and [ElasticFusion](https://github.com/mp3guy/ElasticFusion)

Also work with [ICL-NUIM dataset](https://www.doc.ic.ac.uk/~ahanda/VaFRIC/iclnuim.html) (TUM RGB-D Compatible PNGs)

# Build #
```bash
cd ./pngtoklg
mkdir build
cd build
cmake ..
make
```


# Usage #
## Parameters ##
All parameters are required.
- -w working directory
- -o output file name (the output file will be place under working directory)
- -r associations.txt is in reverse order (rgb)(depth)
- -t TUM format / defualt format is ICL-NUIM
- -s Scale factor in floating point ex. '0.0002'

## Prerequirement##
Should place **associations.txt** under working directory.
About how to generate associations.txt please read "Related files" section.

## Example ##
```bash
./pngtoklg -w ../livingroom_kt0_rs -o liv.klg -s 0.0002 -t
```
After execute the command above, "./livingroom_kt0_rs" folder should have liv.klg file (about 3.2Mb).


## Convert TUM dataset ##
```bash
./pngtoklg -w './tum/rgbd_dataset_freiburg2_360_kidnap/' -o './360_kidnap.klg' -t -s 0.0002
```

Run with ElasticFusion
```bash
./ElasticFusion -l (path to 360_kidnap.klg) -d 12 -c 3 -f
```

## Convert ICL-NUIM dataset
remove -t option which is stand for tum
```bash
./pngtoklg -w './tum/rgbd_dataset_freiburg2_360_kidnap/' -o './360_kidnap.klg' -s 0.0002
```

# Related files #


### rgb.txt format ###
One row contain two informations.
First is time sequence.
Actually the time is not important.  We only need increasing number sequence.
```
timeSequence filePath
```
Sample file content
```
0.033333 ./rgb/scene_00_0000_rs.png
0.066666 ./rgb/scene_00_0001_rs.png
0.099999 ./rgb/scene_00_0002_rs.png
0.133332 ./rgb/scene_00_0003_rs.png
0.166665 ./rgb/scene_00_0004_rs.png
0.199998 ./rgb/scene_00_0005_rs.png
...
```
## associate.py ##
> This code is developed by TUM, which use to associate rgb.txt and depth.txt

Type the following command
```bash
>python associate.py PATH_TO_SEQUENCE/rgb.txt PATH_TO_SEQUENCE/depth.txt > associations.txt
```

Sample file content (TUM RGB-D dataset format)
If you are using ICL-NUIM, the timestamp will be integer number
```
1311868164.363181 rgb/1311868164.363181.png 1311868164.373557 depth/1311868164.373557.png
1311868164.399026 rgb/1311868164.399026.png 1311868164.407784 depth/1311868164.407784.png
1311868164.430940 rgb/1311868164.430940.png 1311868164.437021 depth/1311868164.437021.png
1311868164.463055 rgb/1311868164.463055.png 1311868164.477039 depth/1311868164.477039.png
1311868164.499130 rgb/1311868164.499130.png 1311868164.507698 depth/1311868164.507698.png
1311868164.531025 rgb/1311868164.531025.png 1311868164.539395 depth/1311868164.539395.png
```
