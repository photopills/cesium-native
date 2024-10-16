#!/bin/bash

# Array of paths and URLs
submodules=(
  "extern/glm https://github.com/g-truc/glm.git"
  "extern/uriparser https://github.com/uriparser/uriparser.git"
  "extern/draco https://github.com/google/draco.git"
  "extern/earcut https://github.com/mapbox/earcut.hpp.git"
  "extern/Catch2 https://github.com/catchorg/Catch2.git"
  "extern/GSL https://github.com/microsoft/GSL.git"
  "extern/tinyxml2 https://github.com/leethomason/tinyxml2.git"
  "extern/asyncplusplus https://github.com/Amanieu/asyncplusplus.git"
  "extern/spdlog https://github.com/gabime/spdlog.git"
  "extern/rapidjson https://github.com/Tencent/rapidjson.git"
  "extern/stb https://github.com/nothings/stb.git"
  "extern/cpp-httplib https://github.com/yhirose/cpp-httplib.git"
  "extern/CSPRNG https://github.com/Duthomhas/CSPRNG.git"
  "extern/PicoSHA2 https://github.com/okdshin/PicoSHA2.git"
  "extern/cmake-modules https://github.com/bilke/cmake-modules.git"
  "extern/s2geometry https://github.com/google/s2geometry.git"
  "extern/KTX-Software https://github.com/CesiumGS/KTX-Software.git"
  "extern/libwebp https://github.com/webmproject/libwebp.git"
  "extern/libmorton https://github.com/Forceflow/libmorton.git"
  "extern/expected-lite https://github.com/martinmoene/expected-lite.git"
  "extern/libjpeg-turbo https://github.com/CesiumGS/libjpeg-turbo.git"
  "extern/zlib-ng https://github.com/zlib-ng/zlib-ng.git"
  "extern/meshoptimizer https://github.com/zeux/meshoptimizer.git"
  "extern/CSPRNG https://github.com/Duthomhas/CSPRNG.git"
)

# Loop through the submodules and add them
for submodule in "${submodules[@]}"; do
  # Split the string into path and URL
  path=$(echo $submodule | cut -d' ' -f1)
  url=$(echo $submodule | cut -d' ' -f2)
  
  echo "Adding submodule: $path from $url"
  git submodule add --force "$url" "$path" 
done

# Set branch for KTX-Software
git config -f .gitmodules submodule.extern/KTX-Software.branch cesium-native

# Initialize and update submodules
git submodule update --init --recursive

echo "All submodules added and updated."

