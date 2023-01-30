
echo ""
echo "****************************"
echo "Matisse Installation Script!"
echo "****************************"
echo ""

cd
mkdir matisse_deps_libs
cd matisse_deps_libs/

main_path=$(pwd)
echo "$main_path"

# # for opencv
sudo apt -y install build-essential cmake unzip pkg-config git mercurial
sudo apt -y install libglew-dev

# # for opencv
sudo apt -y install libjpeg-dev libpng-dev libtiff-dev
sudo apt -y install libavcodec-dev libavformat-dev libswscale-dev libv4l-dev
sudo apt -y install libxvidcore-dev libx264-dev
sudo apt -y install libatlas-base-dev gfortran

sudo apt -y install libopenscenegraph-dev

sudo apt -y install libgdal-dev

# # for openMVG
sudo apt -y install libxxf86vm1 libxxf86vm-dev libxi-dev libxrandr-dev
sudo apt -y install graphviz libcereal-dev

# # for openMVS
sudo apt -y install libboost-iostreams-dev libboost-program-options-dev libboost-system-dev libboost-serialization-dev libboost-filesystem-dev
sudo apt -y install libcgal-dev libcgal-qt5-dev 
sudo apt -y install libatlas-base-dev libsuitesparse-dev

# # for mvs texturing
sudo apt -y install libtbb-dev

# # for Matisse
sudo apt -y install ffmpeg
sudo apt -y install libquazip5-dev
sudo apt -y install qt5-default libqt5xmlpatterns5-dev qtscript5-dev
sudo apt -y install libgeographic-dev
sudo apt -y install libkml-dev
sudo apt -y install libimage-exiftool-perl

# # for QSsh
sudo apt -y install libbotan-2-dev

# # build botan2 (!!!!!! if using ubuntu 18.04 you must build last botan from source like this !!!!!)
# #cd ~
# #git clone https://github.com/randombit/botan.git
# #cd botan
# #./configure.py
# #make
# #sudo make install


# Eigen 3.3
git clone https://gitlab.com/libeigen/eigen.git
cd eigen/
git checkout 3.4
mkdir build
cd build/
cmake ..
make -j4 && sudo make install

# # build and install opencv4 only from source for ubuntu 18.04
# #cd ~
# #wget -O opencv.zip https://github.com/opencv/opencv/archive/4.0.0.zip
# #unzip opencv.zip
# #cd opencv-4.0.0
# #mkdir build
# #cd build
# #cmake -D CMAKE_BUILD_TYPE=RELEASE ..
# #make -j4
sudo apt -y install libopencv-dev

# build and install openMVG
cd $main_path
git clone --recursive https://github.com/openMVG/openMVG.git
cd openMVG/
git checkout v2.0
mkdir openMVG_build 
cd openMVG_build
cmake -DCMAKE_BUILD_TYPE=Release -DOpenMVG_BUILD_SHARED:BOOL=ON -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON -DSUITESPARSE:BOOL=ON ../src/
make -j8 && make install

# # build and install openMVS
cd $main_path
git clone https://github.com/cdcseacave/VCG.git vcglib
cd vcglib/
git checkout 88f12f21

cd $main_path
git clone https://github.com/cdcseacave/openMVS.git
cd openMVS/
git checkout v1.1.1
mkdir openMVS_build 
cd openMVS_build
cmake -DCMAKE_BUILD_TYPE=Release -DVCG_ROOT="$main_path/vcglib" -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON ..
make -j4 && make install

# # build mvs-texturing
cd $main_path
git clone https://github.com/neoaurion/mvs-texturing.git
cd mvs-texturing/
mkdir build
cd build/
cmake ..
make -j8
sudo make install


# # build qssh
cd $main_path
git clone https://github.com/neoaurion/QSsh.git
cd QSsh/
mkdir build
cd build/
cmake ..
make -j8
sudo make install

# Install Matisse
cd
mkdir software
cd software
git clone https://github.com/IfremerUnderwater/Matisse
cd Matisse/Src/
mkdir matisse_build
cd matisse_build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j8

sudo ldconfig

cd Run/Release

echo ""
echo "****************************"
echo "Matisse Installed at $(pwd)!"
echo "****************************"
echo ""