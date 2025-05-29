#pragma once

#include "../../external/imgui/imgui.h"
#include "../../external/imgui/rlImGui.h"
#include "../raylib/raylib.h"
#include <string>
#include <vector>

struct AVFormatContext;
struct AVCodecContext;
struct AVStream;
struct SwsContext;
struct AVFrame;

struct UpdateVariables;
struct UpdateParameters;

class ScreenCapture {
public:
    bool exportMemoryFrames = false;
    bool deleteFrames = false;
    bool isExportingFrames = false;
    bool isFunctionRecording = false;
    bool isVideoExportEnabled = true;

    bool isSafeFramesEnabled = true;
    bool isExportFramesEnabled = false;

    bool showSaveConfirmationDialog = false;
    std::string lastVideoPath;

    bool videoHasBeenSaved = false;
    std::string actualSavedVideoFolder;
    std::string actualSavedVideoName;

    bool cancelRecording = false;

    bool screenGrab(RenderTexture2D &myParticlesTexture, UpdateVariables &myVar,
                    UpdateParameters &myParam);

private:
    std::string generateVideoFilename();
    void cleanupFFmpeg();
    void exportFrameToFile(const Image &frame, const std::string &videoFolder,
                           const std::string &videoName, int frameNumber);    void exportMemoryFramesToDisk();
    void discardMemoryFrames();
    void createFramesFolder(const std::string &folderPath);
    void discardRecording();
    void applyButtonStyle(const ImVec4 &baseColor);
    void popButtonStyle();

    int screenshotIndex = 0; // Used for screenshot naming
    std::vector<Image> myFrames; // Used for storing captured frames
    int diskModeFrameIdx = 0; // Used for tracking frames in disk mode
    std::string folderName; // Used for storing the folder name
    std::string outFileName; // Used for storing the output file name
    std::string videoFolder; // Used for storing the video folder path

    AVFormatContext *pFormatCtx = nullptr;
    AVCodecContext *pCodecCtx = nullptr;
    AVStream *pStream = nullptr;
    SwsContext *swsCtx = nullptr;
    AVFrame *frame = nullptr;
    int frameIndex = 0;
};
