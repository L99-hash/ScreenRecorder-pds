#!/bin/bash

echo "####################################"
echo "#         Screen Recoder           #"
echo "#               by                 #"
echo "#    Luca Crupi and Enrico Bravi   #"
echo "####################################"
echo
echo "Start building the project..."
echo

echo "Creating the ./build directory..."
echo

mkdir build
if [ $? -eq 0 ]
then
    echo "Created the ./build direcotory"
else
    echo "ERROR: cannot create the ./build direcotry"
fi
cmake -S . -B build

echo "Moving to ./build direcotory"
echo
cd build
make

if [ $? -eq 0 ]
then
    echo
    echo "Build succeded"
else
    echo
    echo "Build failed"
fi
