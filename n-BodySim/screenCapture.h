#pragma once
#include "raylib.h"
#include <iostream>

class ScreenCapture {
public:
	bool screenGrab();

private:
	int screenshotIndex = 0;
	int frameIndex = 0;
	bool isRecording = false;
};
