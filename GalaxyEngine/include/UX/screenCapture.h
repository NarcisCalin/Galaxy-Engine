#pragma once

#include "../raylib/raylib.h"
#include "../UI/button.h"
#include <vector>

struct UpdateVariables;

class ScreenCapture {
public:
	bool exportMemoryFrames = false;
	bool deleteFrames = false;
	bool isFunctionRecording = false;
	bool isVideoExportEnabled = true;

	bool isSafeFramesEnabled = true;
	bool isExportFramesEnabled = false;

	bool screenGrab(RenderTexture2D& myParticlesTexture, UpdateVariables& myVar);

private:
	int screenshotIndex = 0;
	std::vector<Image> myFrames;
	int diskModeFrameIdx = 0;
	std::string folderName;
};
