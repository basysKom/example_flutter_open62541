#!/bin/bash

# dart sdk required for async callbacks (tested version was a44b4f5)
if [ ! -e "sdk" ]; then
  git clone --depth 1 https://github.com/dart-lang/sdk
else
  echo "Skipping sdk"
fi

if [ ! -e "open62541" ]; then
  # Get and build open62541
  git clone https://github.com/open62541/open62541.git

  cd open62541
  git checkout 1.2 # Tested version was 1.2
  git apply ../open62541-CMakeLists.txt.patch

  # Build open62541 and put install files in a subdirectory (we need the includes)
  mkdir -p build/install_dir && cd build
  cmake -DCMAKE_INSTALL_PREFIX=$(pwd)/install_dir ..
  make -j$(nproc)
  make install
else
  echo "Skipping open62541"
fi

# echo "---------------------------------------------------------------------"
# echo "|For android also apply patch open62541-android-CMakeLists.txt.patch|"
# echo "---------------------------------------------------------------------"

