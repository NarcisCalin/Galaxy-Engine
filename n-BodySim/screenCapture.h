#pragma once
#include "raylib.h"
#include "button.h"
#include <vector>

class ScreenCapture {
public:
	bool exportFrames = false;
	bool deleteFrames = false;
	bool isRecording = false;

	bool screenGrab(RenderTexture2D& myParticlesTexture, bool& isDragging, bool& isMouseNotHoveringUI);

private:
	int screenshotIndex = 0;
	int frameIndex = 0;
	int videoIndex = 0;
	std::vector<Image> myFrames;
};
