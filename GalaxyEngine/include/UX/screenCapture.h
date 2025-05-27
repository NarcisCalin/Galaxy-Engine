#pragma once

#include "../raylib/raylib.h"
#include "../../external/imgui/imgui.h"
#include "../../external/imgui/rlImGui.h"
#include <string>
#include <vector>

// Forward declarations for FFmpeg types
struct AVFormatContext;
struct AVCodecContext;
struct AVStream;
struct SwsContext;
struct AVFrame;

struct UpdateVariables;

class ScreenCapture {
public:
	bool exportMemoryFrames = false;
	bool deleteFrames = false;
	bool isFunctionRecording = false;
	bool isVideoExportEnabled = true;
	

	bool isSafeFramesEnabled = true;
	bool isExportFramesEnabled = false;
	// Added for save confirmation
	bool showSaveConfirmationDialog = false;
	std::string lastVideoPath;
	
	// Added for frame export after save
	bool videoHasBeenSaved = false;
	std::string actualSavedVideoFolder; // The actual folder where video was saved
	std::string actualSavedVideoName;   // The actual name of the saved video
	
	// Added for cancel recording
	bool cancelRecording = false;

	bool screenGrab(RenderTexture2D& myParticlesTexture, UpdateVariables& myVar);

private:
	std::string generateVideoFilename();	// Helper methods
	void cleanupFFmpeg();
	void exportFrameToFile(const Image& frame, const std::string& videoFolder, const std::string& videoName, int frameNumber);
	void createFramesFolder(const std::string& folderPath);
	void applyButtonStyle(const ImVec4& baseColor);
	void popButtonStyle();
	std::string createTempFramesFolder(); // Create temporary folder for safe frames
	void moveTempFramesToFinalFolder(const std::string& finalFolder, const std::string& videoName); // Move temp frames to final location
	
	// Variables for screen capture
	int screenshotIndex = 0;
	std::vector<Image> myFrames;
	int diskModeFrameIdx = 0;
	std::string folderName;
	std::string outFileName;
	std::string tempFramesFolder; // Temporary folder for safe frames during recording
		// FFmpeg context variables
	AVFormatContext* pFormatCtx = nullptr;
	AVCodecContext* pCodecCtx = nullptr;
	AVStream* pStream = nullptr;
	SwsContext* swsCtx = nullptr;
	AVFrame* frame = nullptr;
	int frameIndex = 0;
};
