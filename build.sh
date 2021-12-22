#!/bin/bash

echo "####################################"
echo "#         Screen Recoder           #"
echo "#               by                 #"
echo "#    Luca Crupi and Enrico Bravi   #"
echo "####################################"
echo
echo "Start building the project..."
echo

cmake .
make

if [ $? -eq 0 ]
then
    echo
    echo "Build succeded"
else
    echo
    echo "Build failed"
fi