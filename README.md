# Readme

## TODOs:
- Per-Pool Prioritäten für threads
- Thread pool naming (i.e., set/get string name for thread pool object)
- Thread naming inside pool (e.g., "worker-", then 1st thread is "worker-1", 2nd is "worker2") etc.
- Rename gecko_pool to thread_pool or gecko_thread_pool (conflict with string pool naming)
- Statistics per pool (e.g., min/max/avg/mean run time, idle time, job time)
- Statistics per thread (e.g., run time, idle time, job time,...)
- Method to increase/decrease # of threads in pool
- Error checking (e.g., null check with require_nonnull macro)
- code "Documentation" from HackMd to doxygen code docu in sources

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
