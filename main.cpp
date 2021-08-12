#include <iostream>
#include "ScreenRecorder.h"

int main() {
    ScreenRecorder screenRecorder;

    screenRecorder.openCamera();
    screenRecorder.initOutputFile();
    screenRecorder.captureVideoFrames();
    return 0;
}
