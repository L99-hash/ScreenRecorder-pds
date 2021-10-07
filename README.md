# ScreenRecorder-pds

Screen recorder built using FFmpeg 4.1 library. Project realized for System and Device Programming course of Politecnico di Torino

# Linux
Dependencies:
- libavcodec
- libavformat
- libavfilter
- libavutil
- libavdevice
- libswscale
- libswresample
- libx11

Before build the preject, perform: 
- ```sudo apt-get update```
- ```sudo apt-get install libavcodec-dev libavformat-dev libavfilter-dev libavutil-dev libavdevice-dev libswscale-dev libswresample-dev libx11-dev cmake pkg-config -y```

To build and execute the project:
- move in the project directory
- ```cmake .```
- ```make```
- ```./ScreenRecorder_pds```

### Last test on linux with versions:

- libavutil      56. 51.100 / 56. 51.100
- libavcodec     58. 91.100 / 58. 91.100
- libavformat    58. 45.100 / 58. 45.100
- libavdevice    58. 10.100 / 58. 10.100
- libavfilter    7. 85.100 /  7. 85.100
- libavresample  4.  0.  0 /  4.  0.  0
- libswresample  3.  7.100 /  3.  7.100
- libx11         2: 1.7.2-1

# Develop your own project

To develop your own project, you can use the ScreenRecorder.cpp as wrapper APIs for FFmpeg librarry.

## Documentation

TODO
