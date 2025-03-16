#include "screenCapture.h"

bool ScreenCapture::screenGrab() {

	if (IsKeyPressed(KEY_S)) {
		TakeScreenshot(TextFormat("Screenshot_%i.png", screenshotIndex));
		screenshotIndex++;
	}

	if (IsKeyPressed(KEY_R)) {
		isRecording = !isRecording;
	}
	if (isRecording) {
		TakeScreenshot(TextFormat("SimFrame_%i.png", frameIndex));
		frameIndex++;

	}

	return isRecording;
}
