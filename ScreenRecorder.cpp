//
// Created by enrico on 11/08/21.
//

#include "ScreenRecorder.h"

using namespace std;

ScreenRecorder::ScreenRecorder() {
    avcodec_register_all();
    avdevice_register_all();
}
