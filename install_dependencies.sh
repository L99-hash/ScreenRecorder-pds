#!/bin/bash

echo "Installing dependencies:"
echo
echo "  libavcodec-dev"
echo "  libavformat-dev"
echo "  libavfilter-dev"
echo "  libavutil-dev"
echo "  libavdevice-dev"
echo "  libswscale-dev"
echo "  libswresample-dev"
echo "  libx11-dev"
echo "  cmake"
echo "  pkg-config"
echo

apt-get update
apt-get install libavcodec-dev libavformat-dev libavfilter-dev libavutil-dev libavdevice-dev libswscale-dev libswresample-dev libx11-dev cmake pkg-config -y

if [ $? -eq 0 ]
then
    echo
    echo "Installation succeded"
else
    echo
    echo "Installation failed"
fi