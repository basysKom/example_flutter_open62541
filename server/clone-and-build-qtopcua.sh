#!/bin/bash

QMAKE="$HOME/Qt/5.15.2/gcc_64/bin/qmake"


# Build and install qt opca for Qt 5.15.2
git clone https://github.com/qt/qtopcua.git
cd qtopcua
git checkout v5.15.2
git apply ../*.patch
mkdir build && cd build
$QMAKE ..
make -j$(nproc)
make install

# Build examples
make -j$(nproc) sub-examples
