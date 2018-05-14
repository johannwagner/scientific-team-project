# Readme

## Configure gtest framework



### Ubuntu
```bash
    sudo apt-get install libgtest-dev
```

### ArchLinux
```bash
    pacman -S gtest
```
### Set Location of the library
```bash
cd /usr/src/gtest
sudo cmake CMakeLists.txt
sudo make

sudo cp *.a /usr/local/lib/
sudo cp ../gtest /usr/local/include/gtest -r
```

*Note: currently compiling the project with gcc is broken we suggest that you use clang*
### Compile the Project
```bash
CC=clang CXX=clang++ cmake /path/to/project/.
cd /path/to/project
make
```
