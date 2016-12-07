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
- -w working directory path
- -d depth info file
- -r rgb info file
- -o output file name (the output file will be place under working directory)

## Example ##
```bash
./pngtoklg -w ../livingroom_kt0_rs -d depth.txt -r rgb.txt -o liv.klg
```
After execute the command above, "./livingroom_kt0_rs" folder should have liv.klg file (about 3.2Mb).



# gen.py #
> ICL-NUIM dataset has no rgb.txt and depth.txt files.  This script will generate TUM dataset like rgb.txt and depth.txt from depth and rgb folder in ICL-NUIM dataset..

Type the following command
```bash
>python ./gen.py livingroom_kt0_rs
```
It will generate rgb.txt and depth.txt under livingroom_kt0_rs folder.

## rgb.txt format ##
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
