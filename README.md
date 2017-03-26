# png to klg format convertor for ElasticFusion

Convert [TUM RGB-D png dataset](http://vision.in.tum.de/data/datasets/rgbd-dataset/download#freiburg1_desk) to .klg format for [Kintinuous](https://github.com/mp3guy/Kintinuous) and [ElasticFusion](https://github.com/mp3guy/ElasticFusion)

Also work with [ICL-NUIM dataset](https://www.doc.ic.ac.uk/~ahanda/VaFRIC/iclnuim.html) (TUM RGB-D Compatible PNGs)

# What do I need to build it?

  - Ubuntu 14.04, 15.04 or 16.04 (Though many other linux distros will work fine)
  - CMake
  - Boost
  - zlib
  - libjpeg
  - [OpenCV](http://sourceforge.net/projects/opencvlibrary/files/opencv-unix/2.4.9/opencv-2.4.9.zip)

```bash
sudo apt-get install g++ cmake cmake-gui libboost-all-dev build-essential

wget http://sourceforge.net/projects/opencvlibrary/files/opencv-unix/2.4.9/opencv-2.4.9.zip
unzip opencv-2.4.9.zip
cd opencv-2.4.9.zip
mkdir build
cd build
cmake -D BUILD_NEW_PYTHON_SUPPORT=OFF -D WITH_OPENCL=OFF -D WITH_OPENMP=ON -D INSTALL_C_EXAMPLES=OFF -D BUILD_DOCS=OFF -D BUILD_EXAMPLES=OFF -D WITH_QT=OFF -D WITH_OPENGL=OFF -D WITH_VTK=OFF -D BUILD_PERF_TESTS=OFF -D BUILD_TESTS=OFF -D WITH_CUDA=OFF -D BUILD_opencv_gpu=OFF ..
make -j8
sudo make install
```
  
## Python package
  - numpy

```bash
sudo apt-get install pip
pip install numpy
```


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
Download the file provided by ICL-NUIM.
https://www.doc.ic.ac.uk/~ahanda/VaFRIC/iclnuim.html
(Living Room 'lr kt0') => (TUM RGB-D Compatible PNGs)


```bash
./pngtoklg -w ~/Downloads/living_room_traj0_frei_png -o ~/Downloads/living_room_traj0_frei_png/liv.klg -s 0.0002
```
After execute the command above, "~/Downloads/living_room_traj0_frei_png" folder should have liv.klg file (about 3.2GB).


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
>python associate.py PATH_TO_SEQUENCE/depth.txt PATH_TO_SEQUENCE/rgb.txt > associations.txt
```

Sample file content (TUM RGB-D dataset format)
If you are using ICL-NUIM, the timestamp will be integer number
```
0.033333 ./depth/scene_00_0000_rs.png 0.033333 ./rgb/scene_00_0000_rs.png
0.066666 ./depth/scene_00_0001_rs.png 0.066666 ./rgb/scene_00_0001_rs.png
0.099999 ./depth/scene_00_0002_rs.png 0.099999 ./rgb/scene_00_0002_rs.png
0.133332 ./depth/scene_00_0003_rs.png 0.133332 ./rgb/scene_00_0003_rs.png
0.166665 ./depth/scene_00_0004_rs.png 0.166665 ./rgb/scene_00_0004_rs.png
0.199998 ./depth/scene_00_0005_rs.png 0.199998 ./rgb/scene_00_0005_rs.png
```
