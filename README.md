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

Before build the preject, perform (check that ```install_dependencies.sh``` and ```build.sh``` have execution permission): 
- ```sudo ./install_dependencies.sh```

To build and execute the project:
- ```./build.sh```

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
