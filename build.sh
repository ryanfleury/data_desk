#!/bin/bash

exists()
{
  command -v "$1" >/dev/null 2>&1
}

force_gcc=false
while getopts 'g' opt
do
  case $opt in
    g) force_gcc=true ;;
  esac
done

if [ ! -d "./build" ]; then 
    mkdir build 
fi

pushd build
if exists clang && ! $force_gcc; then 
    echo "$0: Compiling against clang."
    clang ../source/data_desk_main.c -DBUILD_LINUX=1 -DBUILD_WIN32=0 -o ./data_desk -ldl
else 
    echo "$0: Compiling against gcc."
    gcc ../source/data_desk_main.c -DBUILD_LINUX=1 -DBUILD_WIN32=0 -o ./data_desk -ldl
fi
popd