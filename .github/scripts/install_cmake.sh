VERSION=$1
BUILD=$2

wget https://cmake.org/files/v$VERSION/cmake-$VERSION.$BUILD.tar.gz
tar -xzvf cmake-$VERSION.$BUILD.tar.gz
cd cmake-$VERSION.$BUILD/

./bootstrap
make -j$(nproc)
sudo make install
