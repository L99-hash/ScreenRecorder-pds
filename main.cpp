#include <iostream>
#include "ScreenRecorder.h"

int main() {
    ScreenRecorder screenRecorder;

    screenRecorder.openCamera();
    screenRecorder.initOutputFile();
    return 0;
}
