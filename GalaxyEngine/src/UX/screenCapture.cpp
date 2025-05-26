#include "../../include/UX/screenCapture.h"
#include "../../include/parameters.h"
#include <cstdio>
#include <filesystem>
#include <string>
#include <algorithm>
#include <regex>
#include <sstream>
#include <cstring>         
#ifdef _WIN32
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

void ScreenCapture::exportFrameToFile(const Image& frame, const std::string& framesFolder, int frameNumber) {
	Image frameCopy = frame;
	ImageFlipVertical(&frameCopy);
	
	std::string filename = framesFolder + "/Frame_" + std::to_string(frameNumber) + ".png";
	ExportImage(frameCopy, filename.c_str());
	
	UnloadImage(frameCopy);
}

void ScreenCapture::createFramesFolder(const std::string& folderPath) {
	if (!std::filesystem::exists(folderPath)) {
		std::filesystem::create_directory(folderPath);
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

std::string ScreenCapture::generateVideoFilename() {
	// Always use default Video_X pattern, custom naming happens at save time
	int maxNumberFound = 0;
	std::regex videoFileRegex(R"(Video_(\d+)\.mp4)");

	if (std::filesystem::exists("Videos")) {
		for (const auto& entry : std::filesystem::directory_iterator("Videos")) {
			if (entry.is_regular_file()) {
				std::string fileName = entry.path().filename().string();
				std::smatch match;
				if (std::regex_match(fileName, match, videoFileRegex)) {
					int number = std::stoi(match[1].str());
					maxNumberFound = std::max(maxNumberFound, number);
				}
			}
		}
	}

	int nextAvailableNumber = maxNumberFound + 1;
	return "Videos/Video_" + std::to_string(nextAvailableNumber) + ".mp4";
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
			
			// Extract folder name for frame exports
			size_t lastSlash = outFileName.find_last_of('/');
			size_t lastDot = outFileName.find_last_of('.');
			if (lastSlash != std::string::npos && lastDot != std::string::npos) {
				folderName = outFileName.substr(lastSlash + 1, lastDot - lastSlash - 1);
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
			std::filesystem::remove(outFileName);
			printf("Recording cancelled and file deleted: %s\n", outFileName.c_str());
		}

		for (Image& frameImg : myFrames) {
			UnloadImage(frameImg);
		}
		myFrames.clear();
		std::vector<Image>().swap(myFrames);
		diskModeFrameIdx = 0;

		cancelRecording = false;
	}

	if (isFunctionRecording) {
		Image img = LoadImageFromTexture(myParticlesTexture.texture);
		ImageFlipVertical(&img);
		int w = img.width;
		int h = img.height;
		const uint8_t* srcSlices[1] = {
			reinterpret_cast<const uint8_t*>(img.data)
		};
		int srcStride[1] = { 4 * w };

		sws_scale(swsCtx, srcSlices, srcStride, 0, h, frame->data, frame->linesize);
		
		if (frame) {
			if (pStream) {
				frame->pts = av_rescale_q(frameIndex++, pCodecCtx->time_base, pStream->time_base);
			} else {
				frame->pts = frameIndex++;
			}
		}

		avcodec_send_frame(pCodecCtx, frame);

		AVPacket* pkt = av_packet_alloc();
		if (!pkt) {
			printf("Could not allocate packet\n");
			return false;
		}

		while (avcodec_receive_packet(pCodecCtx, pkt) == 0) {
			if (pStream) {
				pkt->stream_index = pStream->index;
				av_interleaved_write_frame(pFormatCtx, pkt);
			}
			av_packet_unref(pkt);
		}

		av_packet_free(&pkt);

		UnloadImage(img);
	}

	if (isFunctionRecording && !isSafeFramesEnabled && isExportFramesEnabled) {
		myFrames.push_back(LoadImageFromTexture(myParticlesTexture.texture));
	}

	float screenW = GetScreenWidth();
	float screenH = GetScreenHeight();

	ImVec2 framesMenuSize = { 400.0f, 200.0f };
	if (myFrames.size() > 0 || diskModeFrameIdx > 0 || isFunctionRecording) {
		ImGui::SetNextWindowSize(framesMenuSize, ImGuiCond_Once);
		ImGui::SetNextWindowPos(ImVec2(screenW - framesMenuSize.x - 10.0f, 10.0f), ImGuiCond_Appearing);

		ImGui::Begin("Recording Menu", nullptr, ImGuiWindowFlags_NoCollapse);

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




		if (myFrames.size() > 0 && !isFunctionRecording && !isSafeFramesEnabled) {

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
	}

	// Save confirmation dialog
	if (showSaveConfirmationDialog) {
		ImGui::SetNextWindowSize(ImVec2(400, 200), ImGuiCond_Always);
		ImGui::SetNextWindowPos(ImVec2(screenW * 0.5f - 200, screenH * 0.5f - 90), ImGuiCond_Appearing);
		ImGui::Begin("Save Recording?", &showSaveConfirmationDialog, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);
		ImGui::PushFont(myVar.robotoMediumFont);
		ImGui::SetWindowFontScale(1.5f);

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
			// Check if user entered a custom name
			std::string customName = std::string(nameBuffer);
			if (!customName.empty()) {
				// Clean the custom name
				customName.erase(std::remove_if(customName.begin(), customName.end(), 
					[](char c) { return c == '<' || c == '>' || c == ':' || c == '"' || 
								 c == '|' || c == '?' || c == '*' || c == '/'; }), customName.end());
				
				if (!customName.empty()) {
					// Add .mp4 extension if not present
					if (customName.length() < 4 || customName.substr(customName.length() - 4) != ".mp4") {
						customName += ".mp4";
					}
					
					std::string newPath = "Videos/" + customName;
					
					// Rename the file if the new name is different
					if (newPath != lastVideoPath && std::filesystem::exists(lastVideoPath)) {
						try {
							std::filesystem::rename(lastVideoPath, newPath);
							printf("Video renamed to: %s\n", newPath.c_str());
						} catch (const std::exception& e) {
							printf("Failed to rename video: %s\n", e.what());
						}
					}
				}
			}
			
			// Clear the buffer for next time
			memset(nameBuffer, 0, sizeof(nameBuffer));
			showSaveConfirmationDialog = false;
		}
		
		ImGui::SameLine();
		
		if (ImGui::Button("Discard", ImVec2(100, 30))) {
			if (std::filesystem::exists(lastVideoPath)) {
				std::filesystem::remove(lastVideoPath);
				printf("Recording discarded: %s\n", lastVideoPath.c_str());
			}
			
			memset(nameBuffer, 0, sizeof(nameBuffer));
			showSaveConfirmationDialog = false;
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
	}

	if (exportMemoryFrames && !isSafeFramesEnabled) {
		isFunctionRecording = false;

		int numFrames = static_cast<int>(myFrames.size());

		if (!std::filesystem::exists("Videos")) {
			std::filesystem::create_directory("Videos");
		}

		std::string filename = generateVideoFilename();
		
		size_t lastSlash = filename.find_last_of('/');
		size_t lastDot = filename.find_last_of('.');
		std::string baseName;
		if (lastSlash != std::string::npos && lastDot != std::string::npos) {
			baseName = filename.substr(lastSlash + 1, lastDot - lastSlash - 1);
		} else {
			baseName = "Video_1";
		}

		std::string framesFolder = "Videos/" + baseName + "_frames";

		createFramesFolder(framesFolder);

#pragma omp parallel for
		for (int i = 0; i < numFrames; ++i) {
			exportFrameToFile(myFrames[i], framesFolder, i);
		}

		myFrames.clear();
		std::vector<Image>().swap(myFrames);
		exportMemoryFrames = false;
		diskModeFrameIdx++;
	}

	if (isFunctionRecording && (isSafeFramesEnabled || isVideoExportEnabled)) {

		if (isSafeFramesEnabled && isExportFramesEnabled) {
			Image frame = LoadImageFromTexture(myParticlesTexture.texture);

			std::string framesFolder = "Videos/" + folderName + "_frames";
			createFramesFolder(framesFolder);

			exportFrameToFile(frame, framesFolder, diskModeFrameIdx);
			UnloadImage(frame);
		}

		diskModeFrameIdx++;
	}

	return isFunctionRecording;
}
