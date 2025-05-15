# This script assumes that all clang-uml dependencies are instaled in C:\clang-uml

param ($Prefix="C:\llvm20", $BuildType="Release")

mkdir _BUILD

cmake -S .. -B .\_BUILD\windows\ -DBOOST_STATIC=ON -DCMAKE_PREFIX_PATH="$Prefix" -Thost=x64

cd .\_BUILD\windows

msbuild .\clang-include-graph.vcxproj -maxcpucount /p:Configuration=Release

cpack -C "Release" -G NSIS64 

cd ..
cd ..