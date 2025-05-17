#include "../../include/UX/screenCapture.h"
#include "../../include/parameters.h"
#include <cstdio>
#include <filesystem>
#include <string>
#include <algorithm>
#include <regex>
#include <sstream>         
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

AVFormatContext* pFormatCtx = nullptr;
std::string outFileName;

bool ScreenCapture::screenGrab(RenderTexture2D& myParticlesTexture, UpdateVariables& myVar) {

	if (IsKeyPressed(KEY_S)) {
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

	static bool recording = false;
	static AVFormatContext* pFormatCtx = nullptr;
	static AVCodecContext* pCodecCtx = nullptr;
	static AVStream* pStream = nullptr;
	static struct SwsContext* swsCtx = nullptr;
	static AVFrame* frame = nullptr;
	static int             frameIndex = 0;


	if (IsKeyPressed(KEY_R)) {
		if (!isFunctionRecording && !isSafeFramesEnabled) {
			for (Image& frame : myFrames) {
				UnloadImage(frame);
			}
			myFrames.clear();
			std::vector<Image>().swap(myFrames);
		}

		if (!isFunctionRecording && (isSafeFramesEnabled || isVideoExportEnabled)) {

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

			outFileName = folderName + "/" + folderName + ".mp4";

			if (!std::filesystem::exists(folderName)) {
				std::cout << "Failed to create folder: " << folderName << std::endl;
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

			avformat_write_header(pFormatCtx, nullptr);

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
			avcodec_send_frame(pCodecCtx, nullptr);

			AVPacket* pkt = av_packet_alloc();
			if (!pkt) {
				printf("Could not allocate packet\n");
				return false;
			}

			while (avcodec_receive_packet(pCodecCtx, pkt) == 0) {
				pkt->stream_index = pStream->index;
				av_interleaved_write_frame(pFormatCtx, pkt);
				av_packet_unref(pkt);
			}

			av_write_trailer(pFormatCtx);
			av_packet_free(&pkt);

			sws_freeContext(swsCtx);
			av_frame_free(&frame);
			avcodec_free_context(&pCodecCtx);
			if (!(pFormatCtx->oformat->flags & AVFMT_NOFILE))
				avio_closep(&pFormatCtx->pb);
			avformat_free_context(pFormatCtx);

			pFormatCtx = nullptr;
			pCodecCtx = nullptr;
			pStream = nullptr;
			swsCtx = nullptr;
			frame = nullptr;
			isFunctionRecording = false;

			printf("Stopped recording. File saved as '%s'\n", outFileName.c_str());
		}
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
		frame->pts = av_rescale_q(frameIndex++, pCodecCtx->time_base, pStream->time_base);

		avcodec_send_frame(pCodecCtx, frame);

		AVPacket* pkt = av_packet_alloc();
		if (!pkt) {
			printf("Could not allocate packet\n");
			return false;
		}

		while (avcodec_receive_packet(pCodecCtx, pkt) == 0) {
			pkt->stream_index = pStream->index;
			av_interleaved_write_frame(pFormatCtx, pkt);
			av_packet_unref(pkt);
		}

		av_packet_free(&pkt);

		UnloadImage(img);
	}

	if (isFunctionRecording && !isSafeFramesEnabled && isExportFramesEnabled) {
		myFrames.push_back(LoadImageFromTexture(myParticlesTexture.texture));
	}

	if (myFrames.size() > 0 && !isFunctionRecording && !isSafeFramesEnabled) {
		Button exportFramesButton({ static_cast<float>(GetScreenWidth()) - 600.0f, 70.0f },
			{ 170.0f, 35.0f }, "Export Frames", true);
		Button deleteFramesButton({ static_cast<float>(GetScreenWidth()) - 600.0f, 110.0f },
			{ 170.0f, 35.0f }, "Discard Frames", true);

		bool isExportFramesButtonHovering = exportFramesButton.buttonLogic(exportMemoryFrames);
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

	if (diskModeFrameIdx > 0 && (isSafeFramesEnabled || isVideoExportEnabled)) {
		DrawText(TextFormat("Rendered Frames: %i", diskModeFrameIdx), GetScreenWidth() - 600, 40, 25, RED);
	}
	else if (myFrames.size() > 0 && !isSafeFramesEnabled) {
		DrawText(TextFormat("Rendered Frames: %i", myFrames.size()), GetScreenWidth() - 600, 40, 25, RED);
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

		if (!isVideoExportEnabled) {
			int nextAvailableNumber = maxNumberFound + 1;
			folderName = "Video_" + std::to_string(nextAvailableNumber);
		}
		else {
			int nextAvailableNumber = maxNumberFound;
			folderName = "Video_" + std::to_string(nextAvailableNumber);
		}

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
		exportMemoryFrames = false;
		diskModeFrameIdx++;
	}

	if (isFunctionRecording && (isSafeFramesEnabled || isVideoExportEnabled)) {

		if (isSafeFramesEnabled && isExportFramesEnabled) {
			Image frame = LoadImageFromTexture(myParticlesTexture.texture);
			ImageFlipVertical(&frame);

			std::string filename = folderName + "/" + folderName + "_Frame_" + std::to_string(diskModeFrameIdx) + ".png";
			ExportImage(frame, filename.c_str());

			UnloadImage(frame);
		}

		diskModeFrameIdx++;
	}

	return isFunctionRecording;
}
