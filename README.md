# NovaVCS

A next-generation version control system built in C++17.

## Build Instructions (Ubuntu)
```bash
sudo apt update
sudo apt install build-essential cmake
mkdir build && cd build
cmake ..
make
```

## Running Tests
```bash
cd build
ctest --output-on-failure
```

## Usage
Initialize a repository:
```bash
./src/nova init
```
