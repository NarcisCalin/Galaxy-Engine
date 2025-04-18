#pragma once

#include "../raylib/raylib.h"
#include "../UI/button.h"
#include <vector>

struct UpdateVariables;

class ScreenCapture {
public:
	bool exportFrames = false;
	bool deleteFrames = false;
	bool isFunctionRecording = false;

	bool isDiskModeEnabled = true;

	bool screenGrab(RenderTexture2D& myParticlesTexture, UpdateVariables& myVar);

private:
	int screenshotIndex = 0;
	std::vector<Image> myFrames;
	int diskModeFrameIdx = 0;
	std::string folderName;
};
