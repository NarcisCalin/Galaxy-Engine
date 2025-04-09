#include "screenCapture.h"

bool ScreenCapture::screenGrab(RenderTexture2D& myParticlesTexture, bool& isDragging, bool& isMouseNotHoveringUI) {

	/*if (IsKeyPressed(KEY_S)) {
		TakeScreenshot(TextFormat("Screenshot_%i.png", screenshotIndex));
		screenshotIndex++;
	}*/

	if (IsKeyPressed(KEY_S)) {
		Image renderImage = LoadImageFromTexture(myParticlesTexture.texture);
		ImageFlipVertical(&renderImage);
		ExportImage(renderImage, TextFormat("Screenshot_%i.png", screenshotIndex));
		UnloadImage(renderImage);
		screenshotIndex++;
	}

	if (IsKeyPressed(KEY_R)) {
		if (!isRecording) {
			myFrames.clear();

		}
		isRecording = !isRecording;
	}

	if (isRecording) {
		myFrames.push_back(LoadImageFromTexture(myParticlesTexture.texture));
	}

	if (myFrames.size() > 0 && !isRecording) {
		Button exportFramesButton({ static_cast<float>(GetScreenWidth()) - 600.0f, 70.0f },
			{ 150.0f, 35.0f }, "Export Frames", true);
		Button deleteFramesButton({ static_cast<float>(GetScreenWidth()) - 600.0f, 110.0f },
			{ 150.0f, 35.0f }, "Delete Frames", true);

		bool isExportFramesButtonHovering = exportFramesButton.buttonLogic(exportFrames);
		bool isDeleteFramesButtonHovering = deleteFramesButton.buttonLogic(deleteFrames);

		DrawText("Might take a while", GetScreenWidth() - 440, 70, 20, RED);

		if (isExportFramesButtonHovering || isDeleteFramesButtonHovering) {
			isDragging = false;
			isMouseNotHoveringUI = false;
		}
	}

	if (myFrames.size() > 0) {
		DrawText(TextFormat("Rendered Frames: %i", myFrames.size()), GetScreenWidth() - 600, 40, 25, RED);
	}

	if (deleteFrames) {
		myFrames.clear();
		deleteFrames = false;
	}

	if (exportFrames) {
		isRecording = false;

		int numFrames = static_cast<int>(myFrames.size());
		int currentVideoIndex = videoIndex;

#pragma omp parallel for
		for (int i = 0; i < numFrames; ++i) {
			Image frame = myFrames[i];

			ImageFlipVertical(&frame);

			char filename[256];
			snprintf(filename, sizeof(filename), "Video%d_Frame_%d.png", currentVideoIndex, i);
			ExportImage(frame, filename);

			UnloadImage(frame);
		}
		
		myFrames.clear();

		exportFrames = false;
		frameIndex = 0;
		videoIndex++;
	}

	return isRecording;
}
