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