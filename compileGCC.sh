#/usr/bin/sh
/bin/cmake3 -D CMAKE_CXX_COMPILER=/home/user/GCC-9.2.0/bin/g++ -H./implementation/ -B./build
cd build
make
echo "Done!"

