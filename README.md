# Readme

## TODOs:
- Per-Pool Priority for threads
- Thread naming inside pool (e.g., "worker-", then 1st thread is "worker-1", 2nd is "worker2") etc.
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

### Compile the Project
```bash
cmake /path/to/project/.
cd /path/to/project
make
```
