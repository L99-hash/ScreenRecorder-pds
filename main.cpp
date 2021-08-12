#include <iostream>
#include "ScreenRecorder.h"

int main() {
    ScreenRecorder screenRecorder;

    screenRecorder.openDevice();
    screenRecorder.initOutputFile();
    screenRecorder.captureVideoFrames();
    return 0;
}
