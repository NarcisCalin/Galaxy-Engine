#include "../../include/UX/screenCapture.h"
#include <filesystem>
#include <string>
#include <algorithm>
#include <regex>
#include "../../include/parameters.h"

bool ScreenCapture::screenGrab(RenderTexture2D& myParticlesTexture, UpdateVariables& myVar) {

	if (IsKeyPressed(KEY_S)) {
		if (!std::filesystem::exists("Screenshots")) {
			std::filesystem::create_directory("Screenshots");
		}

		int nextAvailableIndex = 0;
		for (const auto& entry : std::filesystem::directory_iterator("Screenshots")) {

			std::string filename = entry.path().filename().string();
			if (filename.rfind("Screenshot_", 0) == 0 && filename.find(".png") != std::string::npos) {

				size_t startPos = filename.find_last_of('_') + 1;
				size_t endPos = filename.find(".png");
				int index = std::stoi(filename.substr(startPos, endPos - startPos));

				if (index >= nextAvailableIndex) {
					nextAvailableIndex = index + 1;
				}
			}
		}

		Image renderImage = LoadImageFromTexture(myParticlesTexture.texture);
		ImageFlipVertical(&renderImage);

		std::string screenshotPath = "Screenshots/Screenshot_" + std::to_string(nextAvailableIndex) + ".png";
		ExportImage(renderImage, screenshotPath.c_str());

		UnloadImage(renderImage);
		screenshotIndex++;
	}


	if (IsKeyPressed(KEY_R)) {
		if (!isFunctionRecording && !isDiskModeEnabled) {
			for (Image& frame : myFrames) {
				UnloadImage(frame);
			}
			myFrames.clear();
			std::vector<Image>().swap(myFrames);
		}
		else if (!isFunctionRecording && isDiskModeEnabled) {

			diskModeFrameIdx = 0;

			int maxNumberFound = 0;
			std::regex folderRegex(R"(Video_(\d+))");

			for (const auto& entry : std::filesystem::directory_iterator(std::filesystem::current_path())) {
				if (entry.is_directory()) {
					std::string folderName = entry.path().filename().string();
					std::smatch match;
					if (std::regex_match(folderName, match, folderRegex)) {
						int number = std::stoi(match[1].str());
						maxNumberFound = std::max(maxNumberFound, number);
					}
				}
			}

			int nextAvailableNumber = maxNumberFound + 1;
			folderName = "Video_" + std::to_string(nextAvailableNumber);

			std::filesystem::create_directory(folderName);
		}

		isFunctionRecording = !isFunctionRecording;
	}

	if (isFunctionRecording && !isDiskModeEnabled) {
		myFrames.push_back(LoadImageFromTexture(myParticlesTexture.texture));
	}

	if (myFrames.size() > 0 && !isFunctionRecording && !isDiskModeEnabled) {
		Button exportFramesButton({ static_cast<float>(GetScreenWidth()) - 600.0f, 70.0f },
			{ 170.0f, 35.0f }, "Export Frames", true);
		Button deleteFramesButton({ static_cast<float>(GetScreenWidth()) - 600.0f, 110.0f },
			{ 170.0f, 35.0f }, "Discard Frames", true);

		bool isExportFramesButtonHovering = exportFramesButton.buttonLogic(exportFrames);
		bool isDeleteFramesButtonHovering = deleteFramesButton.buttonLogic(deleteFrames);

		DrawText("Might take a while", GetScreenWidth() - 420, 70, 20, RED);

		if (isExportFramesButtonHovering || isDeleteFramesButtonHovering) {
			myVar.isMouseNotHoveringUI = false;
			myVar.isDragging = false;
		}
		else {
			myVar.isMouseNotHoveringUI = true;
		}
	}

	if (diskModeFrameIdx > 0 && isDiskModeEnabled) {
		DrawText(TextFormat("Rendered Frames: %i", diskModeFrameIdx), GetScreenWidth() - 600, 40, 25, RED);
	}
	else if (myFrames.size() > 0 && !isDiskModeEnabled) {
		DrawText(TextFormat("Rendered Frames: %i", myFrames.size()), GetScreenWidth() - 600, 40, 25, RED);
	}

	if (!isFunctionRecording && diskModeFrameIdx > 0 && isDiskModeEnabled) {
		diskModeFrameIdx = 0;
	}

	if (deleteFrames) {
		for (Image& frame : myFrames) {
			UnloadImage(frame);
		}
		myFrames.clear();
		std::vector<Image>().swap(myFrames);
		deleteFrames = false;
	}

	if (exportFrames && !isDiskModeEnabled) {
		isFunctionRecording = false;

		int numFrames = static_cast<int>(myFrames.size());

		int maxNumberFound = 0;
		std::regex folderRegex(R"(Video_(\d+))");

		for (const auto& entry : std::filesystem::directory_iterator(std::filesystem::current_path())) {
			if (entry.is_directory()) {
				std::string folderName = entry.path().filename().string();
				std::smatch match;
				if (std::regex_match(folderName, match, folderRegex)) {
					int number = std::stoi(match[1].str());
					maxNumberFound = std::max(maxNumberFound, number);
				}
			}
		}

		int nextAvailableNumber = maxNumberFound + 1;
		folderName = "Video_" + std::to_string(nextAvailableNumber);

		std::filesystem::create_directory(folderName);

		#pragma omp parallel for
				for (int i = 0; i < numFrames; ++i) {
					Image frame = myFrames[i];
					ImageFlipVertical(&frame);
		
					std::string filename = folderName + "/" + folderName + "_Frame_" + std::to_string(i) + ".png";
					ExportImage(frame, filename.c_str());
		
					UnloadImage(frame);
				}

		myFrames.clear();
		std::vector<Image>().swap(myFrames);
		exportFrames = false;
		diskModeFrameIdx++;
	}

	if (isFunctionRecording && isDiskModeEnabled) {

		Image frame = LoadImageFromTexture(myParticlesTexture.texture);
		ImageFlipVertical(&frame);

		std::string filename = folderName + "/" + folderName + "_Frame_" + std::to_string(diskModeFrameIdx) + ".png";
		ExportImage(frame, filename.c_str());

		UnloadImage(frame);

		diskModeFrameIdx++;
	}

	return isFunctionRecording;
}
