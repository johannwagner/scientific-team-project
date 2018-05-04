# Readme

## Configure gtest framework

### Ubuntu

```bash
    sudo apt-get install libgtest-dev

    cd /usr/src/gtest
    sudo cmake CMakeLists.txt
    sudo make

    sudo cp *.a /usr/local/lib/
    sudo cp ../gtest /user/local/include/gtest -r
```

### ArchLinux
```bash
    pacman -S gtest
    
    cd /usr/src/gtest
    sudo cmake CMakeLists.txt
    sudo make

    sudo cp *.a /usr/local/lib/
    sudo cp ../gtest /usr/local/include/gtest -r
    
    #To compile simply direct to the cloned directory and
    cmake .
    make
```
