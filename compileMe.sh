#/usr/bin/sh
cmake -D CMAKE_CXX_COMPILER=/home/vy/GCC-9.2.0-Result/bin/g++ -H./implementation/ -B./build
cd build
make
cd ..
echo "Done!"
