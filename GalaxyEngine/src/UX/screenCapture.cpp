#include "../../include/UX/screenCapture.h"
#include "../../include/parameters.h"
#include <cstdio>
#include <filesystem>
#include <string>
#include <algorithm>
#include <regex>
#include <sstream>
#include <cstring>
#include <chrono>
#include <cmath>

// Prevent Windows macro conflicts with std::min/max and raylib
#ifdef _WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#define NOGDI       // Excludes GDI defines like Rectangle
#define NOUSER      // Excludes USER defines like ShowCursor
#include <windows.h>
#include <psapi.h>
#include <io.h>          
#include <fcntl.h>    
#define popen  _popen
#define pclose _pclose
#define MODE_BINARY _O_BINARY
#else
#include <unistd.h>  
#define MODE_BINARY 0
#endif

#ifdef _WIN32
// Windows-specific includes with relative paths
extern "C" {
#include "../../external/ffmpeg/include/libavcodec/avcodec.h"
#include "../../external/ffmpeg/include/libavformat/avformat.h"
#include "../../external/ffmpeg/include/libavutil/imgutils.h"
#include "../../external/ffmpeg/include/libavutil/avutil.h"
#include "../../external/ffmpeg/include/libavutil/opt.h"
#include "../../external/ffmpeg/include/libswscale/swscale.h"
}
#else
// Linux/Unix includes using system paths
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include <libavutil/frame.h>
}
#endif

void ScreenCapture::cleanupFFmpeg() {
	if (pCodecCtx) {
		avcodec_send_frame(pCodecCtx, nullptr);

		AVPacket* pkt = av_packet_alloc();
		if (pkt) {
			while (avcodec_receive_packet(pCodecCtx, pkt) == 0) {
				av_packet_unref(pkt);
			}
			av_packet_free(&pkt);
		}
	}

	if (swsCtx) {
		sws_freeContext(swsCtx);
		swsCtx = nullptr;
	}
	
	if (frame) {
		av_frame_free(&frame);
		frame = nullptr;
	}
	
	if (pCodecCtx) {
		avcodec_free_context(&pCodecCtx);
		pCodecCtx = nullptr;
	}
	
	if (pFormatCtx) {
		if (pFormatCtx->oformat && !(pFormatCtx->oformat->flags & AVFMT_NOFILE)) {
			avio_closep(&pFormatCtx->pb);
		}
		avformat_free_context(pFormatCtx);
		pFormatCtx = nullptr;
	}
	
	pStream = nullptr;
	frameIndex = 0;
}

void ScreenCapture::exportFrameToFile(const Image& frame, const std::string& videoFolder, const std::string& videoName, int frameNumber) {
	// Check if the video folder exists before attempting export
	if (!std::filesystem::exists(videoFolder)) {
		printf("Warning: Frame export folder does not exist: %s\n", videoFolder.c_str());
		return;
	}
	
	// Create a proper deep copy of the image to avoid double-free issues
	Image frameCopy = ImageCopy(frame);
	ImageFlipVertical(&frameCopy);
	
	std::string filename = videoFolder + "/" + videoName + "_" + std::to_string(frameNumber) + ".png";
	
	// Try to export the frame with error handling
	try {
		ExportImage(frameCopy, filename.c_str());
	} catch (...) {
		printf("Error: Failed to export frame to: %s\n", filename.c_str());
	}
	
	// Clean up the copy we created
	UnloadImage(frameCopy);
}

void ScreenCapture::createFramesFolder(const std::string& folderPath) {
	if (!std::filesystem::exists(folderPath)) {
		try {
			std::filesystem::create_directories(folderPath);
		} catch (const std::exception& e) {
			printf("Error creating folder %s: %s\n", folderPath.c_str(), e.what());
		}
	}
}

void ScreenCapture::applyButtonStyle(const ImVec4& baseColor) {
	ImGui::PushStyleColor(ImGuiCol_Button, baseColor);
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(baseColor.x + 0.1f, baseColor.y + 0.1f, baseColor.z + 0.1f, baseColor.w));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(baseColor.x - 0.1f, baseColor.y - 0.1f, baseColor.z - 0.1f, baseColor.w));
}

void ScreenCapture::popButtonStyle() {
	ImGui::PopStyleColor(3);
}

std::string ScreenCapture::createTempFramesFolder() {
	// Create unique temporary folder using timestamp
	auto now = std::chrono::system_clock::now();
	auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
	
	std::string tempFolder = "Videos/temp_recording_" + std::to_string(timestamp);
	createFramesFolder(tempFolder);
	
	return tempFolder;
}

void ScreenCapture::moveTempFramesToFinalFolder(const std::string& finalFolder, const std::string& videoName) {
	if (tempFramesFolder.empty() || !std::filesystem::exists(tempFramesFolder)) {
		return;
	}
	
	// Ensure final folder exists
	createFramesFolder(finalFolder);
	
	try {
		// Move all frame files from temp folder to final folder, renaming them
		int frameIndex = 0;
		for (const auto& entry : std::filesystem::directory_iterator(tempFramesFolder)) {
			if (entry.is_regular_file() && entry.path().extension() == ".png") {
				std::string finalFramePath = finalFolder + "/" + videoName + "_" + std::to_string(frameIndex) + ".png";
				std::filesystem::rename(entry.path(), finalFramePath);
				frameIndex++;
			}
		}
		
		// Remove temporary folder
		std::filesystem::remove_all(tempFramesFolder);
		tempFramesFolder.clear();
		
		printf("Moved %d frames to final folder: %s\n", frameIndex, finalFolder.c_str());
	} catch (const std::exception& e) {
		printf("Error moving temp frames: %s\n", e.what());
	}
}

std::string ScreenCapture::generateVideoFilename() {
	// Generate folder and filename pattern: Videos/Video_X/Video_X.mp4
	int maxNumberFound = 0;
	std::regex videoFolderRegex(R"(Video_(\d+))");

	if (std::filesystem::exists("Videos")) {
		for (const auto& entry : std::filesystem::directory_iterator("Videos")) {
			if (entry.is_directory()) {
				std::string folderName = entry.path().filename().string();
				std::smatch match;				if (std::regex_match(folderName, match, videoFolderRegex)) {
					int number = std::stoi(match[1].str());
					if (number > maxNumberFound) {
						maxNumberFound = number;
					}
				}
			}
		}
	}

	int nextAvailableNumber = maxNumberFound + 1;
	std::string videoName = "Video_" + std::to_string(nextAvailableNumber);
	return "Videos/" + videoName + "/" + videoName + ".mp4";
}

bool ScreenCapture::screenGrab(RenderTexture2D& myParticlesTexture, UpdateVariables& myVar) {

	if (IO::handleShortcut(KEY_S)) {
		if (!std::filesystem::exists("Screenshots")) {
			std::filesystem::create_directory("Screenshots");
		}

		int nextAvailableIndex = 0;
		for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator("Screenshots")) {

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
	
	
	// There is a bug running the game on X11 on linux where the R key doesn't work
	// on windowed mode. Possibly an upstream issue with either X11 or FFMPEG.
	// TODO: Investigate this bug further.
	if (IO::handleShortcut(KEY_R)) {
		if (!isFunctionRecording && !isSafeFramesEnabled) {
			for (Image& frame : myFrames) {
				UnloadImage(frame);
			}
			myFrames.clear();
			std::vector<Image>().swap(myFrames);
		}
		if (!isFunctionRecording && (isSafeFramesEnabled || isVideoExportEnabled)) {

			diskModeFrameIdx = 0;

			// Create Videos directory if it doesn't exist
			if (!std::filesystem::exists("Videos")) {
				std::filesystem::create_directory("Videos");
			}

			// Generate filename based on custom name or default pattern
			outFileName = generateVideoFilename();
			
			// Create the video folder structure: Videos/Video_X/
			size_t lastSlash = outFileName.find_last_of('/');
			std::string videoFolder = outFileName.substr(0, lastSlash);
			if (!std::filesystem::exists(videoFolder)) {
				std::filesystem::create_directories(videoFolder);
			}
			
			// Extract folder name for frame exports (just the Video_X part)
			size_t secondToLastSlash = outFileName.find_last_of('/', lastSlash - 1);
			if (secondToLastSlash != std::string::npos) {
				folderName = outFileName.substr(secondToLastSlash + 1, lastSlash - secondToLastSlash - 1);
			} else {
				folderName = "Video_1"; // fallback
			}
		}

		if (!isFunctionRecording) {
			int w = GetScreenWidth();
			int h = GetScreenHeight();

			if (avformat_alloc_output_context2(&pFormatCtx, nullptr, nullptr, outFileName.c_str()) < 0) {
				printf("Could not alloc output context\n");
				return false;
			}

			const AVCodec* codec = avcodec_find_encoder(AV_CODEC_ID_H264);
			if (!codec) {
				printf("H.264 codec not found\n");
				return false;
			}

			pStream = avformat_new_stream(pFormatCtx, codec);
			pCodecCtx = avcodec_alloc_context3(codec);
			pCodecCtx->codec_id = AV_CODEC_ID_H264;
			pCodecCtx->width = w;
			pCodecCtx->height = h;
			pCodecCtx->time_base = AVRational{ 1,60 };
			pCodecCtx->framerate = AVRational{ 24,1 };
			pCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
			pCodecCtx->bit_rate = 256 * 1000 * 1000;
			pCodecCtx->gop_size = 12;
			av_opt_set(pCodecCtx->priv_data, "preset", "medium", 0);
			av_opt_set(pCodecCtx->priv_data, "crf", "23", 0);

			if (avcodec_open2(pCodecCtx, codec, nullptr) < 0) {
				printf("Could not open codec\n");
				return false;
			}
			avcodec_parameters_from_context(pStream->codecpar, pCodecCtx);

			if (!(pFormatCtx->oformat->flags & AVFMT_NOFILE)) {
				if (avio_open(&pFormatCtx->pb, outFileName.c_str(), AVIO_FLAG_WRITE) < 0) {
					printf("Could not open output file\n");
					return false;
				}
			}
			// Write the stream header
			if (avformat_write_header(pFormatCtx, nullptr) < 0) {
				printf("Could not write header\n");
				return false;
			}

			frame = av_frame_alloc();
			frame->format = pCodecCtx->pix_fmt;
			frame->width = w;
			frame->height = h;
			av_frame_get_buffer(frame, 0);

			swsCtx = sws_getContext(
				w, h, AV_PIX_FMT_RGBA,
				w, h, AV_PIX_FMT_YUV420P,
				SWS_BILINEAR, nullptr, nullptr, nullptr
			);			
			printf("Started recording to '%s'\n", outFileName.c_str());
			isFunctionRecording = true;
			frameIndex = 0;

			// Reset frame export availability for new recording
			videoHasBeenSaved = false;
			actualSavedVideoFolder.clear();
			actualSavedVideoName.clear();

			// Create temporary folder for safe frames export during recording
			if (isSafeFramesEnabled && isExportFramesEnabled) {
				tempFramesFolder = createTempFramesFolder();
				printf("Created temporary frames folder: %s\n", tempFramesFolder.c_str());
			}

			return true;
		}
		else {
			av_write_trailer(pFormatCtx);
			cleanupFFmpeg();
			isFunctionRecording = false;

			// Show save confirmation dialog
			lastVideoPath = outFileName;
			showSaveConfirmationDialog = true;

			printf("Stopped recording. File saved as '%s'\\n", outFileName.c_str());
		}
	}
	// Handle cancel recording request
	if (cancelRecording && isFunctionRecording) {
		cleanupFFmpeg();
		isFunctionRecording = false;

		if (std::filesystem::exists(outFileName)) {
			try {
				std::filesystem::remove(outFileName);
				printf("Recording cancelled and file deleted: %s\n", outFileName.c_str());
			} catch (const std::exception& e) {
				printf("Warning: Failed to delete cancelled recording: %s\n", e.what());
			}
		}
		for (Image& frameImg : myFrames) {
			UnloadImage(frameImg);
		}
		myFrames.clear();
		std::vector<Image>().swap(myFrames);
		diskModeFrameIdx = 0;
		// Clean up temporary frames folder if it exists
		if (!tempFramesFolder.empty() && std::filesystem::exists(tempFramesFolder)) {
			try {
				std::filesystem::remove_all(tempFramesFolder);
				printf("Temporary frames folder cleaned up: %s\n", tempFramesFolder.c_str());
			} catch (const std::exception& e) {
				printf("Warning: Failed to clean up temporary frames: %s\n", e.what());
			}
			tempFramesFolder.clear();
		}

		// Reset frame export availability since recording was cancelled
		videoHasBeenSaved = false;
		actualSavedVideoFolder.clear();
		actualSavedVideoName.clear();

		cancelRecording = false;
	}	if (isFunctionRecording) {
		// Validate FFmpeg contexts before use
		if (!pCodecCtx || !pFormatCtx || !swsCtx || !frame) {
			printf("Error: FFmpeg contexts not properly initialized\n");
			return false;
		}
		
		// Single texture access point - prevents race conditions
		Image img = LoadImageFromTexture(myParticlesTexture.texture);
		
		// Validate image data
		if (!img.data) {
			printf("Error: Failed to load image from texture\n");
			return isFunctionRecording;
		}
		
		ImageFlipVertical(&img);
		
		// Handle video recording
		int w = img.width;
		int h = img.height;
		const uint8_t* srcSlices[1] = {
			reinterpret_cast<const uint8_t*>(img.data)
		};
		int srcStride[1] = { 4 * w };

		int result = sws_scale(swsCtx, srcSlices, srcStride, 0, h, frame->data, frame->linesize);
		if (result < 0) {
			printf("Error: sws_scale failed\n");
			UnloadImage(img);
			return false;
		}
		
		if (frame) {
			if (pStream) {
				frame->pts = av_rescale_q(frameIndex++, pCodecCtx->time_base, pStream->time_base);
			} else {
				frame->pts = frameIndex++;
			}
		}

		int sendResult = avcodec_send_frame(pCodecCtx, frame);
		if (sendResult < 0) {
			printf("Warning: avcodec_send_frame failed with error %d\n", sendResult);
		}

		AVPacket* pkt = av_packet_alloc();
		if (!pkt) {
			printf("Could not allocate packet\n");
			UnloadImage(img);
			return false;
		}

		while (avcodec_receive_packet(pCodecCtx, pkt) == 0) {
			if (pStream) {
				pkt->stream_index = pStream->index;
				int writeResult = av_interleaved_write_frame(pFormatCtx, pkt);
				if (writeResult < 0) {
					printf("Warning: av_interleaved_write_frame failed with error %d\n", writeResult);
				}
			}
			av_packet_unref(pkt);
		}

		av_packet_free(&pkt);
		
		// Handle frame export operations using the same image data
		if (!isSafeFramesEnabled && isExportFramesEnabled) {
			// Memory mode: store a copy of the image
			Image frameCopy = ImageCopy(img);
			ImageFlipVertical(&frameCopy); // Flip back for proper storage
			myFrames.push_back(frameCopy);
		}
				if (isSafeFramesEnabled && isExportFramesEnabled) {
			// Safe frames mode: export to temporary folder during recording
			if (!tempFramesFolder.empty()) {
				Image frameForExport = ImageCopy(img);
				ImageFlipVertical(&frameForExport); // Flip back for proper export
				
				// Export to temporary folder with simple numbering
				std::string tempFramePath = tempFramesFolder + "/frame_" + std::to_string(diskModeFrameIdx) + ".png";
				ExportImage(frameForExport, tempFramePath.c_str());
				UnloadImage(frameForExport);
			}
		}
		
		// Increment frame counter for disk mode
		if (isSafeFramesEnabled || isVideoExportEnabled) {
			diskModeFrameIdx++;
		}
		
		UnloadImage(img);
	}
	float screenW = GetScreenWidth();
	float screenH = GetScreenHeight();
	ImVec2 framesMenuSize = { 400.0f, 200.0f };
	if (myFrames.size() > 0 || diskModeFrameIdx > 0 || isFunctionRecording) {
		ImGui::SetNextWindowSize(framesMenuSize, ImGuiCond_Once);
		// Position in bottom-left to avoid overlapping with other UI elements
		ImGui::SetNextWindowPos(ImVec2(20.0f, screenH - framesMenuSize.y - 20.0f), ImGuiCond_Appearing);

		ImGui::Begin("Recording Menu", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);

		ImGui::PushFont(myVar.robotoMediumFont);

		ImGui::SetWindowFontScale(1.5f);

		if (diskModeFrameIdx > 0 && (isSafeFramesEnabled || isVideoExportEnabled)) {
			float recordedSeconds = static_cast<float>(diskModeFrameIdx) / 60.0f;
			ImGui::TextColored(ImVec4(0.8f, 0.0f, 0.0f, 1.0f), "Frames: %d (%.2f s)", diskModeFrameIdx, recordedSeconds);
		}
		else if (myFrames.size() > 0 && !isSafeFramesEnabled) {
			float recordedSeconds = static_cast<float>(myFrames.size()) / 60.0f;
			ImGui::TextColored(ImVec4(0.8f, 0.0f, 0.0f, 1.0f), "Frames: %d (%.2f s)", static_cast<int>(myFrames.size()), recordedSeconds);
		}

		if (isFunctionRecording) {
			ImGui::Separator();
			
			// Red cancel button
			applyButtonStyle(ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
			
			if (ImGui::Button("Cancel Recording", ImVec2(ImGui::GetContentRegionAvail().x, 40.0f))) {
				cancelRecording = true;
			}
			
			popButtonStyle();
			
			if (ImGui::IsItemHovered()) {
				ImGui::SetTooltip("Stop recording and discard the video file");
			}
		}

		if (myFrames.size() > 0 && !isFunctionRecording && !isSafeFramesEnabled && videoHasBeenSaved) {

			ImVec4& exportCol = exportMemoryFrames ? myVar.buttonEnabledColor : myVar.buttonDisabledColor;
			applyButtonStyle(exportCol);

			if (ImGui::Button("Export Frames", ImVec2(ImGui::GetContentRegionAvail().x, 40.0f))) {
				exportMemoryFrames = !exportMemoryFrames;
			}
			popButtonStyle();

			ImVec4& discardCol = deleteFrames ? myVar.buttonEnabledColor : myVar.buttonDisabledColor;
			applyButtonStyle(discardCol);

			if (ImGui::Button("Discard Frames", ImVec2(ImGui::GetContentRegionAvail().x, 40.0f))) {
				deleteFrames = !deleteFrames;
			}
			popButtonStyle();

			std::string warning = "EXPORTING MIGHT TAKE A WHILE";

			float windowWidth = ImGui::GetWindowSize().x;
			float textWidth = ImGui::CalcTextSize(warning.c_str()).x;

			ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
			ImGui::TextColored(ImVec4(0.9f, 0.0f, 0.0f, 1.0f), "%s", warning.c_str());


		}
		ImGui::PopFont();
		ImGui::End();
	}	// Save confirmation dialog
	if (showSaveConfirmationDialog) {
		ImGui::SetNextWindowSize(ImVec2(400, 200), ImGuiCond_Always);
		ImGui::SetNextWindowPos(ImVec2(screenW * 0.5f - 200, screenH * 0.5f - 90), ImGuiCond_Appearing);		
		ImGui::Begin("Save Recording?", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);
		ImGui::PushFont(myVar.robotoMediumFont);
		ImGui::SetWindowFontScale(1.5f);

		// Add title since we removed the title bar
		ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Save Recording?");
		ImGui::Separator();

		ImGui::Text("Do you want to save the recording?");
		ImGui::Text("Current: %s", lastVideoPath.c_str());
		
		ImGui::Separator();
		
		// Custom name input
		ImGui::Text("Custom Name (optional):");
		static char nameBuffer[256] = "";
		ImGui::InputText("##CustomVideoName", nameBuffer, sizeof(nameBuffer));
		
		if (ImGui::IsItemHovered()) {
			ImGui::SetTooltip("Leave empty to keep current name");
		}
		
		ImGui::Separator();
		if (ImGui::Button("Save", ImVec2(100, 30))) {
			// Ensure no recording is active before performing file operations
			if (isFunctionRecording) {			printf("Warning: Cannot rename video while recording is active\n");
			} else {
				// Check if user entered a custom name and process all operations
				std::string customName = std::string(nameBuffer);
				std::string finalVideoPath = lastVideoPath;
				std::string newFolderPath;
				std::string newVideoPath;
				
				if (!customName.empty()) {
					// Clean the custom name
					customName.erase(std::remove_if(customName.begin(), customName.end(), 
						[](char c) { return c == '<' || c == '>' || c == ':' || c == '"' || 
									 c == '|' || c == '?' || c == '*' || c == '/'; }), customName.end());
					
					if (!customName.empty()) {
						// Remove .mp4 extension if present to get clean name
						if (customName.length() >= 4 && customName.substr(customName.length() - 4) == ".mp4") {
							customName = customName.substr(0, customName.length() - 4);
						}
						
						// Create new folder structure: Videos/customName/customName.mp4
						newFolderPath = "Videos/" + customName;
						newVideoPath = newFolderPath + "/" + customName + ".mp4";
						finalVideoPath = newVideoPath;
						
						// Create the new folder if it doesn't exist
						if (!std::filesystem::exists(newFolderPath)) {
							try {
								std::filesystem::create_directories(newFolderPath);
							} catch (const std::exception& e) {
								printf("Error creating folder %s: %s\n", newFolderPath.c_str(), e.what());
								// Clear buffer and close dialog on error
								memset(nameBuffer, 0, sizeof(nameBuffer));
								showSaveConfirmationDialog = false;
								return isFunctionRecording;
							}
						}
						
						// Move the video file to the new location with error handling
						if (newVideoPath != lastVideoPath && std::filesystem::exists(lastVideoPath)) {
							try {
								// Check if target file already exists
								if (std::filesystem::exists(newVideoPath)) {
									printf("Warning: Target file already exists: %s\n", newVideoPath.c_str());
								} else {
									std::filesystem::rename(lastVideoPath, newVideoPath);
									printf("Video renamed to: %s\n", newVideoPath.c_str());
									
									// Move any existing frame files as well
									size_t lastSlash = lastVideoPath.find_last_of('/');
									if (lastSlash != std::string::npos) {
										std::string oldFolder = lastVideoPath.substr(0, lastSlash);
										std::string oldVideoName = lastVideoPath.substr(lastSlash + 1);
										// Remove .mp4 extension to get the base name
										if (oldVideoName.length() >= 4 && oldVideoName.substr(oldVideoName.length() - 4) == ".mp4") {
											oldVideoName = oldVideoName.substr(0, oldVideoName.length() - 4);
										}
										
										// Move frame files if they exist
										try {
											for (const auto& entry : std::filesystem::directory_iterator(oldFolder)) {
												if (entry.is_regular_file()) {
													std::string filename = entry.path().filename().string();
													if (filename.find(oldVideoName + "_") == 0 && filename.find(".png") != std::string::npos) {
														// This is a frame file, move it
														std::string newFramePath = newFolderPath + "/" + filename;
														// Update the filename to use new video name
														size_t underscorePos = filename.find('_');
														if (underscorePos != std::string::npos) {
															std::string frameNumber = filename.substr(underscorePos);
															std::string newFrameFilename = customName + frameNumber;
															newFramePath = newFolderPath + "/" + newFrameFilename;
														}
														std::filesystem::rename(entry.path(), newFramePath);
													}
												}
											}
											
											// Remove old folder if it's now empty (and different from new folder)
											if (oldFolder != newFolderPath && std::filesystem::exists(oldFolder) && std::filesystem::is_empty(oldFolder)) {
												std::filesystem::remove(oldFolder);
											}
										} catch (const std::exception& e) {
											printf("Warning: Error moving frame files: %s\n", e.what());
										}
									}
								}
							} catch (const std::exception& e) {
								printf("Failed to rename video: %s\n", e.what());
							}
						}
					}
				}
				
				// Handle moving temporary frames to final folder
				if (!tempFramesFolder.empty()) {
					// Extract folder and video name from final path
					size_t lastSlash = finalVideoPath.find_last_of('/');
					if (lastSlash != std::string::npos) {
						std::string finalFolder = finalVideoPath.substr(0, lastSlash);
						std::string videoFileName = finalVideoPath.substr(lastSlash + 1);
						// Remove .mp4 extension to get video name
						if (videoFileName.length() >= 4 && videoFileName.substr(videoFileName.length() - 4) == ".mp4") {
							videoFileName = videoFileName.substr(0, videoFileName.length() - 4);
						}
						
						// Move temp frames to final folder
						moveTempFramesToFinalFolder(finalFolder, videoFileName);
					}
				}				// Set the actual saved video information for frame export
				size_t finalLastSlash = finalVideoPath.find_last_of('/');
				if (finalLastSlash != std::string::npos) {
					actualSavedVideoFolder = finalVideoPath.substr(0, finalLastSlash);
					actualSavedVideoName = finalVideoPath.substr(finalLastSlash + 1);
					// Remove .mp4 extension to get video name
					if (actualSavedVideoName.length() >= 4 && actualSavedVideoName.substr(actualSavedVideoName.length() - 4) == ".mp4") {
						actualSavedVideoName = actualSavedVideoName.substr(0, actualSavedVideoName.length() - 4);
					}
				}
				
				// Mark that video has been saved (enables frame export dialog)
				videoHasBeenSaved = true;
				
				// Clear the buffer for next time
				memset(nameBuffer, 0, sizeof(nameBuffer));
				showSaveConfirmationDialog = false;
			}
		}
		
		ImGui::SameLine();
				if (ImGui::Button("Discard", ImVec2(100, 30))) {
			// Ensure no recording is active before performing file operations
			if (isFunctionRecording) {
				printf("Warning: Cannot discard video while recording is active\n");
			} else {
				if (std::filesystem::exists(lastVideoPath)) {
					try {
						// Remove the entire folder containing the video and any frames
						size_t lastSlash = lastVideoPath.find_last_of('/');
						if (lastSlash != std::string::npos) {
							std::string videoFolder = lastVideoPath.substr(0, lastSlash);
							if (std::filesystem::exists(videoFolder)) {
								std::filesystem::remove_all(videoFolder);
								printf("Recording and associated files discarded: %s\n", videoFolder.c_str());
							}
						} else {
							// Fallback to just removing the video file
							std::filesystem::remove(lastVideoPath);
							printf("Recording discarded: %s\n", lastVideoPath.c_str());
						}
					} catch (const std::exception& e) {
						printf("Error discarding video: %s\n", e.what());
					}				}
						// Clean up temporary frames folder if it exists
				if (!tempFramesFolder.empty() && std::filesystem::exists(tempFramesFolder)) {
					try {
						std::filesystem::remove_all(tempFramesFolder);
						printf("Temporary frames folder discarded: %s\n", tempFramesFolder.c_str());
						tempFramesFolder.clear();
					} catch (const std::exception& e) {
						printf("Error discarding temporary frames: %s\n", e.what());
					}
				}
				
				// Reset frame export availability since video was discarded
				videoHasBeenSaved = false;
				actualSavedVideoFolder.clear();
				actualSavedVideoName.clear();
				
				memset(nameBuffer, 0, sizeof(nameBuffer));
				showSaveConfirmationDialog = false;
			}
		}

		ImGui::PopFont();
		
		ImGui::End();
	}

	if (!isFunctionRecording && diskModeFrameIdx > 0 && (isSafeFramesEnabled || isVideoExportEnabled)) {
		diskModeFrameIdx = 0;
	}
	if (deleteFrames) {
		for (Image& frame : myFrames) {
			UnloadImage(frame);
		}
		myFrames.clear();
		std::vector<Image>().swap(myFrames);
		deleteFrames = false;
		
		// Reset frame export availability since frames were discarded
		videoHasBeenSaved = false;
		actualSavedVideoFolder.clear();
		actualSavedVideoName.clear();
	}
	if (exportMemoryFrames && !isSafeFramesEnabled) {
		// Prevent recording from being active during export to avoid race conditions
		if (isFunctionRecording) {
			printf("Warning: Cannot export frames while recording is active\n");
			exportMemoryFrames = false;
			return isFunctionRecording;
		}

		int numFrames = static_cast<int>(myFrames.size());
		
		// Check if we have frames to export
		if (numFrames == 0) {
			printf("Warning: No frames to export\n");
			exportMemoryFrames = false;
			return isFunctionRecording;
		}

		if (!std::filesystem::exists("Videos")) {
			std::filesystem::create_directory("Videos");		}

		// Use the actual saved video folder and name instead of generating new ones
		std::string framesFolder = actualSavedVideoFolder;
		std::string baseName = actualSavedVideoName;

		// Ensure folder exists before export
		createFramesFolder(framesFolder);
				// Validate folder creation succeeded
		if (!std::filesystem::exists(framesFolder)) {
			printf("Error: Failed to create frames folder: %s\n", framesFolder.c_str());
			exportMemoryFrames = false;
			return isFunctionRecording;
		}		// Export frames using deep copy (safe method)
		printf("Exporting %d frames to: %s\n", numFrames, framesFolder.c_str());
		
		// Start timing
		auto startTime = std::chrono::high_resolution_clock::now();
		
		#pragma omp parallel for
		for (int i = 0; i < numFrames; ++i) {
			// Validate frame before export
			if (myFrames[i].data != nullptr) {
				exportFrameToFile(myFrames[i], framesFolder, baseName, i);
			} else {
				printf("Warning: Skipping invalid frame at index %d\n", i);
			}
		}
		
		// End timing and calculate results
		auto endTime = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
		auto durationSeconds = std::chrono::duration_cast<std::chrono::duration<double>>(endTime - startTime);
		
		// Display timing results
		printf("Frame export completed successfully!\n");
		printf("Export Duration: %.3f seconds (%.0f ms)\n", durationSeconds.count(), static_cast<double>(duration.count()));
		printf("Average Time per Frame: %.3f ms\n", static_cast<double>(duration.count()) / numFrames);
		printf("Frames per Second: %.2f FPS\n", numFrames / durationSeconds.count());
		// Clean up frames after export
		for (Image& frame : myFrames) {
			if (frame.data != nullptr) {
				UnloadImage(frame);
			}	
		}
		myFrames.clear();
		std::vector<Image>().swap(myFrames);
		exportMemoryFrames = false;
		diskModeFrameIdx++;
		
		// Reset frame export availability after successful export
		videoHasBeenSaved = false;
		actualSavedVideoFolder.clear();
		actualSavedVideoName.clear();
	}

	return isFunctionRecording;
}
