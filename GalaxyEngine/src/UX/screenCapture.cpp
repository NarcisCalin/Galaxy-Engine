#include "UI/UI.h"

#include "UX/screenCapture.h"

#include "parameters.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>
}

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

void ScreenCapture::exportFrameToFile(const Image& frame,
	const std::string& videoFolder,
	const std::string& videoName,
	int frameNumber) {

	if (!std::filesystem::exists(videoFolder)) {
		printf("Warning: Frame export folder does not exist: %s\n",
			videoFolder.c_str());
		return;
	}

	Image frameCopy = ImageCopy(frame);
	ImageFlipVertical(&frameCopy);

	std::string filename = videoFolder + "/" + videoName + "_" +
		std::to_string(frameNumber) + ".png";

	try {
		ExportImage(frameCopy, filename.c_str());
	}
	catch (...) {
		printf("Error: Failed to export frame to: %s\n", filename.c_str());
	}

	UnloadImage(frameCopy);
}

void ScreenCapture::exportMemoryFramesToDisk() {
	if (myFrames.empty()) {
		printf("No frames in memory to export.\n");
		return;
	}

	if (actualSavedVideoFolder.empty() || actualSavedVideoName.empty()) {
		printf("Error: No saved video information available for frame "
			"export.\n");
		return;
	}
	createFramesFolder(actualSavedVideoFolder);
	printf("Exporting %d frames to %s with base name %s...\n",
		static_cast<int>(myFrames.size()), actualSavedVideoFolder.c_str(),
		actualSavedVideoName.c_str());

	auto startTime = std::chrono::high_resolution_clock::now();

	int exportedCount = 0;
	const int totalFrames = static_cast<int>(myFrames.size());

#pragma omp parallel for reduction(+ : exportedCount) schedule(static)
	for (int i = 0; i < totalFrames; ++i) {
		try {
			exportFrameToFile(myFrames[i], actualSavedVideoFolder,
				actualSavedVideoName, i);
			exportedCount++;

			if (i % 100 == 0) {
#pragma omp critical
				{
					printf("Exported frame %d/%d (%.1f%%)\n", i + 1, totalFrames,
						(static_cast<float>(i + 1) / totalFrames) * 100.0f);
				}
			}
		}
		catch (...) {
#pragma omp critical
			{
				printf("Warning: Failed to export frame %d\n", i);
			}
		}
	}

	auto endTime = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
		endTime - startTime);

	printf("Successfully exported %d out of %d frames in %.2f seconds.\n",
		exportedCount, static_cast<int>(myFrames.size()),
		static_cast<double>(duration.count()) / 1000.0);
}

void ScreenCapture::discardMemoryFrames() {
	if (myFrames.empty()) {
		printf("No frames in memory to discard.\n");
		return;
	}

	printf("Discarding %d frames from memory...\n",
		static_cast<int>(myFrames.size()));

	for (Image& frame : myFrames) {
		UnloadImage(frame);
	}
	myFrames.clear();
	std::vector<Image>().swap(myFrames);

	printf("All frames discarded from memory.\n");
}

void ScreenCapture::createFramesFolder(const std::string& folderPath) {
	if (!std::filesystem::exists(folderPath)) {
		try {
			std::filesystem::create_directories(folderPath);
		}
		catch (const std::exception& e) {
			printf("Error creating folder %s: %s\n", folderPath.c_str(), e.what());
		}
	}
}

void ScreenCapture::discardRecording() {
	// Clean up video folder and any files if it was created
	if (!this->videoFolder.empty()) {
		try {
			// Delete individual frame files if safe frames mode was enabled
			if (isSafeFramesEnabled && isExportFramesEnabled && !this->folderName.empty()) {
				std::string framePrefix = this->folderName + "_frame_";
				int deletedFrameCount = 0;

				if (std::filesystem::exists(this->videoFolder)) {
					for (const auto& entry : std::filesystem::directory_iterator(this->videoFolder)) {
						if (entry.is_regular_file()) {
							std::string frameFileName = entry.path().filename().string();
							if (frameFileName.rfind(framePrefix, 0) == 0 &&
								frameFileName.length() > framePrefix.length() &&
								frameFileName.substr(frameFileName.length() - 4) == ".png") {
								try {
									std::filesystem::remove(entry.path());
									deletedFrameCount++;
								}
								catch (const std::filesystem::filesystem_error& e_frame) {
									printf("Warning: Failed to delete frame %s: %s\n",
										entry.path().string().c_str(), e_frame.what());
								}
							}
						}
					}

					if (deletedFrameCount > 0) {
						printf("Deleted %d frame files from cancelled recording\n", deletedFrameCount);
					}
				}
			}

			// Remove the entire video folder since recording was cancelled
			if (std::filesystem::exists(this->videoFolder)) {
				try {
					std::filesystem::remove_all(this->videoFolder);
					printf("Removed video folder: %s\n", this->videoFolder.c_str());
				}
				catch (const std::filesystem::filesystem_error& e_folder) {
					printf("Warning: Failed to remove video folder %s: %s\n",
						this->videoFolder.c_str(), e_folder.what());
				}
			}

		}
		catch (const std::filesystem::filesystem_error& e_dir) {
			printf("Warning: Failed to access video folder for cleanup: %s\n", e_dir.what());
		}
	}
}

std::string ScreenCapture::generateVideoFilename() {
	int maxNumberFound = 0;
	const std::string prefix = "Video_";

	if (std::filesystem::exists("Videos")) {
		for (const auto& entry : std::filesystem::directory_iterator("Videos")) {
			if (entry.is_directory()) {
				std::string folderName = entry.path().filename().string();

				if (folderName.compare(0, prefix.size(), prefix) == 0) {
					const char* numberPart = folderName.c_str() + prefix.size();
					char* endPtr = nullptr;

					double value = std::strtod(numberPart, &endPtr);

					if (endPtr != numberPart && *endPtr == '\0') {
						int number = static_cast<int>(value);
						if (number > maxNumberFound) {
							maxNumberFound = number;
						}
					}
				}
			}
		}
	}

	int nextAvailableNumber = maxNumberFound + 1;
	std::string videoName = prefix + std::to_string(nextAvailableNumber);
	return "Videos/" + videoName + "/" + videoName + ".mp4";
}

bool ScreenCapture::screenGrab(RenderTexture2D& myParticlesTexture,
	UpdateVariables& myVar,
	UpdateParameters& myParam) {

	if (IO::shortcutPress(KEY_S)) {
		if (!std::filesystem::exists("Screenshots")) {
			std::filesystem::create_directory("Screenshots");
		}

		int nextAvailableIndex = 0;
		for (const std::filesystem::directory_entry& entry :
			std::filesystem::directory_iterator("Screenshots")) {

			std::string filename = entry.path().filename().string();
			if (filename.rfind("Screenshot_", 0) == 0 &&
				filename.find(".png") != std::string::npos) {

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

		std::string screenshotPath =
			"Screenshots/Screenshot_" + std::to_string(nextAvailableIndex) + ".png";
		ExportImage(renderImage, screenshotPath.c_str());

		UnloadImage(renderImage);
		screenshotIndex++;
	}

	if (IO::shortcutPress(KEY_R) && !showSaveConfirmationDialog) {
		if (!isFunctionRecording && !isSafeFramesEnabled) {
			for (Image& frame : myFrames) {
				UnloadImage(frame);
			}
			myFrames.clear();
			std::vector<Image>().swap(myFrames);
		}
		if (!isFunctionRecording && isVideoExportEnabled) {

			diskModeFrameIdx = 0;

			if (!std::filesystem::exists("Videos")) {
				std::filesystem::create_directory("Videos");
			}

			outFileName = generateVideoFilename();

			size_t lastSlash = outFileName.find_last_of('/');
			this->videoFolder = outFileName.substr(0, lastSlash);
			if (!std::filesystem::exists(this->videoFolder)) {
				std::filesystem::create_directories(this->videoFolder);
			}

			size_t secondToLastSlash = outFileName.find_last_of('/', lastSlash - 1);
			if (secondToLastSlash != std::string::npos) {
				folderName = outFileName.substr(secondToLastSlash + 1,
					lastSlash - secondToLastSlash - 1);
			}
			else {
				folderName = "Video_1";
			}
		}

		if (!isFunctionRecording) {
			int w = GetScreenWidth();
			int h = GetScreenHeight();

			if (avformat_alloc_output_context2(&pFormatCtx, nullptr, nullptr,
				outFileName.c_str()) < 0) {
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
			pCodecCtx->time_base = AVRational{ 1, 60 };
			pCodecCtx->framerate = AVRational{ 24, 1 };
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
				if (avio_open(&pFormatCtx->pb, outFileName.c_str(), AVIO_FLAG_WRITE) <
					0) {
					printf("Could not open output file\n");
					return false;
				}
			}

			if (avformat_write_header(pFormatCtx, nullptr) < 0) {
				printf("Could not write header\n");
				return false;
			}

			frame = av_frame_alloc();
			frame->format = pCodecCtx->pix_fmt;
			frame->width = w;
			frame->height = h;
			av_frame_get_buffer(frame, 0);

			swsCtx = sws_getContext(w, h, AV_PIX_FMT_RGBA, w, h, AV_PIX_FMT_YUV420P,
				SWS_BILINEAR, nullptr, nullptr, nullptr);
			printf("Started recording to '%s'\n", outFileName.c_str());
			isFunctionRecording = true;
			frameIndex = 0;

			videoHasBeenSaved = false;
			actualSavedVideoFolder.clear();
			actualSavedVideoName.clear();

			return true;
		}
		else {
			av_write_trailer(pFormatCtx);
			cleanupFFmpeg();
			isFunctionRecording = false;

			lastVideoPath = outFileName;
			showSaveConfirmationDialog = true;

			if (myVar.pauseAfterRecording) {
				myVar.isTimePlaying = false;
			}
			if (myVar.cleanSceneAfterRecording) {
				myParam.pParticles.clear();
				myParam.rParticles.clear();
			}

			printf("Stopped recording. File saved as '%s'\\n", outFileName.c_str());
		}
	}  if (cancelRecording && isFunctionRecording) {
		cleanupFFmpeg();
		isFunctionRecording = false;

		if (std::filesystem::exists(outFileName)) {
			try {
				std::filesystem::remove(outFileName);
				printf("Recording cancelled and file deleted: "
					"%s\n",
					outFileName.c_str());
			}
			catch (const std::exception& e) {
				printf("Warning: Failed to delete cancelled "
					"recording: %s\n",
					e.what());
			}
		}

		// Clean up frame files and folder if safe frames mode was enabled
		discardRecording();

		for (Image& frameImg : myFrames) {
			UnloadImage(frameImg);
		}
		myFrames.clear();
		std::vector<Image>().swap(myFrames);

		diskModeFrameIdx = 0;

		videoHasBeenSaved = false;
		actualSavedVideoFolder.clear();
		actualSavedVideoName.clear();

		// Clear recording state variables
		lastVideoPath.clear();
		this->videoFolder.clear();
		this->folderName.clear();

		cancelRecording = false;
	}
	if (isFunctionRecording) {

		if (!pCodecCtx || !pFormatCtx || !swsCtx || !frame) {
			printf("Error: FFmpeg contexts not properly "
				"initialized\n");
			return false;
		}

		Image img = LoadImageFromTexture(myParticlesTexture.texture);

		if (!img.data) {
			printf("Error: Failed to load image from texture\n");
			return isFunctionRecording;
		}

		ImageFlipVertical(&img);

		int w = img.width;
		int h = img.height;
		const uint8_t* srcSlices[1] = { reinterpret_cast<const uint8_t*>(img.data) };
		int srcStride[1] = { 4 * w };

		int result = sws_scale(swsCtx, srcSlices, srcStride, 0, h, frame->data,
			frame->linesize);
		if (result < 0) {
			printf("Error: sws_scale failed\n");
			UnloadImage(img);
			return false;
		}

		if (frame) {
			if (pStream) {
				frame->pts = av_rescale_q(frameIndex++, pCodecCtx->time_base,
					pStream->time_base);
			}
			else {
				frame->pts = frameIndex++;
			}
		}

		if (myVar.recordingTimeLimit > 0.0f) {
			float recordedSeconds =
				static_cast<float>(frameIndex) / pCodecCtx->time_base.den;
			if (recordedSeconds >= myVar.recordingTimeLimit) {
				printf("Recording time limit reached (%.1f "
					"seconds). Stopping "
					"recording.\n",
					myVar.recordingTimeLimit);

				av_write_trailer(pFormatCtx);
				cleanupFFmpeg();
				isFunctionRecording = false;

				lastVideoPath = outFileName;
				showSaveConfirmationDialog = true;

				if (myVar.pauseAfterRecording) {
					myVar.isTimePlaying = false;
				}
				if (myVar.cleanSceneAfterRecording) {
					myParam.pParticles.clear();
					myParam.rParticles.clear();
				}
				UnloadImage(img);
				return isFunctionRecording;
			}
		}

		int sendResult = avcodec_send_frame(pCodecCtx, frame);
		if (sendResult < 0) {
			printf("Warning: avcodec_send_frame failed with error "
				"%d\n",
				sendResult);
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
					printf("Warning: "
						"av_interleaved_write_frame "
						"failed with "
						"error %d\n",
						writeResult);
				}
			}
			av_packet_unref(pkt);
		}

		av_packet_free(&pkt);
		if (!isSafeFramesEnabled && isExportFramesEnabled) {

			Image frameCopy = ImageCopy(img);
			ImageFlipVertical(&frameCopy);
			myFrames.push_back(frameCopy);
		}
		if (isSafeFramesEnabled && isExportFramesEnabled) {

			if (!this->videoFolder.empty() && !this->folderName.empty()) {
				Image frameForExport = ImageCopy(img);

				std::string safeFramePath = this->videoFolder + "/" + this->folderName +
					"_frame_" +
					std::to_string(diskModeFrameIdx) + ".png";
				ExportImage(frameForExport, safeFramePath.c_str());
				UnloadImage(frameForExport);
			}
			else {
				printf("Warning: videoFolder or folderName is "
					"empty, cannot "
					"save safe "
					"frame.\\n");
			}
		}

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
		float yPosition = 30.0f;
		ImGui::SetNextWindowPos(
			ImVec2(screenW * 0.5f - framesMenuSize.x * 0.5f, yPosition),
			ImGuiCond_Appearing);

		ImGui::Begin("Recording Menu", nullptr,
			ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);

		ImGui::PushFont(myVar.robotoMediumFont);

		ImGui::SetWindowFontScale(1.5f);

		if (diskModeFrameIdx > 0 && (isSafeFramesEnabled || isVideoExportEnabled)) {
			float recordedSeconds = static_cast<float>(diskModeFrameIdx) / 60.0f;
			ImGui::TextColored(ImVec4(0.8f, 0.0f, 0.0f, 1.0f), "Frames: %d (%.2f s)",
				diskModeFrameIdx, recordedSeconds);
		}
		else if (myFrames.size() > 0 && !isSafeFramesEnabled) {
			float recordedSeconds = static_cast<float>(myFrames.size()) / 60.0f;
			ImGui::TextColored(ImVec4(0.8f, 0.0f, 0.0f, 1.0f), "Frames: %d (%.2f s)",
				static_cast<int>(myFrames.size()), recordedSeconds);
		}    if (isFunctionRecording) {
			ImGui::Separator();

			if (ImGui::Button("End Recording",
				ImVec2(ImGui::GetContentRegionAvail().x, 40.0f))) {

				av_write_trailer(pFormatCtx);
				cleanupFFmpeg();
				isFunctionRecording = false;

				lastVideoPath = outFileName;
				showSaveConfirmationDialog = true;

				if (myVar.pauseAfterRecording) {
					myVar.isTimePlaying = false;
				}
				if (myVar.cleanSceneAfterRecording) {
					myParam.pParticles.clear();
					myParam.rParticles.clear();
				}

				printf("Recording ended via button. File saved "
					"as '%s'\n",
					outFileName.c_str());
			}

			if (ImGui::IsItemHovered()) {
				ImGui::SetTooltip("Stop recording and save the video file");
			}

			ImGui::PushStyleColor(ImGuiCol_Button, UpdateVariables::colButtonRedActive);
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, UpdateVariables::colButtonRedActiveHover);
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, UpdateVariables::colButtonRedActivePress);

			if (ImGui::Button("Cancel Recording",
				ImVec2(ImGui::GetContentRegionAvail().x, 40.0f))) {
				cancelRecording = true;
			}

			ImGui::PopStyleColor(3);

			if (ImGui::IsItemHovered()) {
				ImGui::SetTooltip("Stop recording and discard "
					"the video file");
			}
		}
		if (myFrames.size() > 0 && !isFunctionRecording && !isSafeFramesEnabled &&
			isExportFramesEnabled && videoHasBeenSaved) {

			// Show different text based on export status
			// Note: Not Working, the exporting blocks the UI
			// TODO: if we manage to unblock the UI, we can use this
			std::string buttonText =
				isExportingFrames ? "Exporting..." : "Export Frames";

			// Disable the button if export is in progress
			if (isExportingFrames) {
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha,
					ImGui::GetStyle().Alpha * 0.5f);
			}

			bool buttonClicked = ImGui::Button(
				buttonText.c_str(), ImVec2(ImGui::GetContentRegionAvail().x, 40.0f));

			if (isExportingFrames) {
				ImGui::PopStyleVar();
			}
			// Only process click if not currently exporting
			if (buttonClicked && !isExportingFrames) {
				exportMemoryFrames = !exportMemoryFrames;
			}

			// Add tooltip for better user feedback
			if (ImGui::IsItemHovered()) {
				if (isExportingFrames) {
					ImGui::SetTooltip("Export in progress, "
						"please wait...");
				}
				else {
					ImGui::SetTooltip("Export frames from memory to disk "
						"as PNG files");
				}
			}

			// Show different text based on export status
			std::string discardButtonText =
				isExportingFrames ? "Export in progress..." : "Discard Frames";

			if (isExportingFrames) {
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha,
					ImGui::GetStyle().Alpha * 0.5f);
			}

			ImGui::PushStyleColor(ImGuiCol_Button, UpdateVariables::colButtonRedActive);
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, UpdateVariables::colButtonRedActiveHover);
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, UpdateVariables::colButtonRedActivePress);

			bool discardButtonClicked =
				ImGui::Button(discardButtonText.c_str(),
					ImVec2(ImGui::GetContentRegionAvail().x, 40.0f));

			ImGui::PopStyleColor(3);
			if (isExportingFrames) {
				ImGui::PopStyleVar();
			}

			// Only process click if not currently exporting
			if (discardButtonClicked && !isExportingFrames) {
				deleteFrames = !deleteFrames;
			}

			// Add tooltip for discard button
			if (ImGui::IsItemHovered()) {
				if (isExportingFrames) {
					ImGui::SetTooltip("Cannot discard frames while "
						"export is in progress");
				}
				else {
					ImGui::SetTooltip("Discard all frames from memory "
						"without saving");
				}
			}
		}
		ImGui::PopFont();
		ImGui::End();
	}
	// Process frame export/discard actions
	if (exportMemoryFrames && videoHasBeenSaved &&
		!actualSavedVideoFolder.empty() && !actualSavedVideoName.empty() &&
		!isExportingFrames) {
		isExportingFrames = true; // Set flag to indicate export is in progress
		exportMemoryFramesToDisk();

		// After export completes, clean up and close dialog
		exportMemoryFrames = false; // Reset the flag after processing
		isExportingFrames = false;  // Clear the export in progress flag

		// Clear frames from memory and close the recording menu
		discardMemoryFrames();
	}
	if (deleteFrames && !myFrames.empty() && !isExportingFrames) {
		discardMemoryFrames();
		deleteFrames = false; // Reset the flag after processing
	}

	if (showSaveConfirmationDialog) {
		ImGui::SetNextWindowSize(ImVec2(400, 200), ImGuiCond_Always);
		ImGui::SetNextWindowPos(ImVec2(screenW * 0.5f - 200, screenH * 0.5f - 90),
			ImGuiCond_Appearing);
		ImGui::Begin("Save Recording?", nullptr,
			ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoTitleBar);
		ImGui::PushFont(myVar.robotoMediumFont);
		ImGui::SetWindowFontScale(1.5f);

		ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Save Recording?");
		ImGui::Separator();

		ImGui::Text("Do you want to save the recording?");
		ImGui::Text("Current: %s", lastVideoPath.c_str());

		ImGui::Separator();

		ImGui::Text("Custom Name (optional):");
		static char nameBuffer[256] = "";
		ImGui::InputText("##CustomVideoName", nameBuffer, sizeof(nameBuffer));

		if (ImGui::IsItemHovered()) {
			ImGui::SetTooltip("Leave empty to keep current name");
		}    ImGui::Separator();

		if (ImGui::Button("Save", ImVec2(100, 30))) {

			if (isFunctionRecording) {
				printf("Warning: Cannot rename video while "
					"recording is "
					"active\\n");
			}
			else {
				std::string customNameInput = std::string(nameBuffer);
				std::string finalVideoPath = lastVideoPath;

				if (!customNameInput.empty()) {
					std::string cleanedCustomName = customNameInput;

					cleanedCustomName.erase(
						std::remove_if(cleanedCustomName.begin(), cleanedCustomName.end(),
							[](char c) {
								return c == '<' || c == '>' || c == ':' ||
									c == '"' || c == '|' || c == '?' ||
									c == '*' || c == '/' || c == '\\';
							}),
						cleanedCustomName.end());

					if (cleanedCustomName.length() >= 4 &&
						cleanedCustomName.substr(cleanedCustomName.length() - 4) ==
						".mp4") {
						cleanedCustomName =
							cleanedCustomName.substr(0, cleanedCustomName.length() - 4);
					}

					if (!cleanedCustomName.empty() &&
						cleanedCustomName != this->folderName) {
						std::string oldBaseName = this->folderName;
						std::string oldFolderPath = this->videoFolder;
						std::string oldVideoFileNameWithExt = oldBaseName + ".mp4";

						std::string parentDir = "Videos";
						size_t parentPathEndPos = oldFolderPath.find_last_of('/');
						if (parentPathEndPos != std::string::npos) {
							parentDir = oldFolderPath.substr(0, parentPathEndPos);
						}

						std::string newBaseName = cleanedCustomName;
						std::string newFolderPath = parentDir + "/" + newBaseName;
						std::string newVideoFileNameWithExt = newBaseName + ".mp4";

						bool folderRenamedSuccessfully = false;

						if (oldFolderPath != newFolderPath &&
							std::filesystem::exists(oldFolderPath)) {
							try {
								if (std::filesystem::exists(newFolderPath)) {
									printf("Wa"
										"rn"
										"in"
										"g:"
										" T"
										"ar"
										"ge"
										"t "
										"fo"
										"ld"
										"er"
										" %"
										"s "
										"fo"
										"r "
										"re"
										"na"
										"me"
										" a"
										"lr"
										"ea"
										"dy"
										" e"
										"xi"
										"st"
										"s."
										" "
										"Ab"
										"or"
										"ti"
										"ng"
										" r"
										"en"
										"am"
										"e "
										"to"
										" p"
										"re"
										"ve"
										"nt"
										" d"
										"at"
										"a "
										"lo"
										"ss"
										"."
										"\\"
										"n",
										newFolderPath.c_str());
								}
								else {
									std::filesystem::rename(oldFolderPath, newFolderPath);
									printf("Fo"
										"ld"
										"er"
										" r"
										"en"
										"am"
										"ed"
										" f"
										"ro"
										"m "
										"%s"
										" t"
										"o "
										"%s"
										"\\"
										"n",
										oldFolderPath.c_str(), newFolderPath.c_str());
									this->videoFolder = newFolderPath;

									lastVideoPath =
										this->videoFolder + "/" + oldVideoFileNameWithExt;
									folderRenamedSuccessfully = true;
								}
							}
							catch (const std::filesystem::filesystem_error& e_folder) {
								printf("Error "
									"renaming "
									"folder %s "
									"to %s: "
									"%s\\n",
									oldFolderPath.c_str(), newFolderPath.c_str(),
									e_folder.what());
							}
						}
						else if (oldFolderPath == newFolderPath) {
							folderRenamedSuccessfully = true;
						}
						else if (!std::filesystem::exists(oldFolderPath)) {
							printf("Error: Original "
								"folder %s not "
								"found. "
								"Cannot perform "
								"rename "
								"operations.\\n",
								oldFolderPath.c_str());
						}

						if (folderRenamedSuccessfully) {

							std::string currentVideoFilePath = lastVideoPath;
							std::string targetVideoFilePath =
								this->videoFolder + "/" + newVideoFileNameWithExt;

							if (currentVideoFilePath != targetVideoFilePath &&
								std::filesystem::exists(currentVideoFilePath)) {
								try {
									if (std::filesystem::exists(targetVideoFilePath) &&
										currentVideoFilePath != targetVideoFilePath) {
										printf("Warning: Target video file %s "
											"already exists. "
											"Overwriting.\\n",
											targetVideoFilePath.c_str());
										std::filesystem::remove(targetVideoFilePath);
									}
									std::filesystem::rename(currentVideoFilePath,
										targetVideoFilePath);
									printf("Vi"
										"de"
										"o "
										"fi"
										"le"
										" r"
										"en"
										"am"
										"ed"
										" t"
										"o "
										"%s"
										"\\"
										"n",
										targetVideoFilePath.c_str());
									lastVideoPath = targetVideoFilePath;
								}
								catch (const std::filesystem::filesystem_error& e_video) {
									printf("Er"
										"ro"
										"r "
										"re"
										"na"
										"mi"
										"ng"
										" v"
										"id"
										"eo"
										" f"
										"il"
										"e "
										"fr"
										"om"
										" %"
										"s "
										"to"
										" %"
										"s:"
										" %"
										"s"
										"\\"
										"n",
										currentVideoFilePath.c_str(),
										targetVideoFilePath.c_str(), e_video.what());
								}
							}
							else if (!std::filesystem::exists(currentVideoFilePath)) {
								printf("Warning: "
									"Video "
									"file %s "
									"not found "
									"for "
									"renaming."
									"\\n",
									currentVideoFilePath.c_str());
							}
							finalVideoPath = lastVideoPath;

							if (isSafeFramesEnabled && isExportFramesEnabled &&
								oldBaseName != newBaseName) {
								std::string oldFramePrefix = oldBaseName + "_frame"
									"_";
								std::string newFramePrefix = newBaseName + "_frame"
									"_";
								try {
									for (const auto& entry :
										std::filesystem::directory_iterator(this->videoFolder)) {
										if (entry.is_regular_file()) {
											std::string currentFrameFileName =
												entry.path().filename().string();
											if (currentFrameFileName.rfind(oldFramePrefix, 0) == 0 &&
												currentFrameFileName.length() >
												oldFramePrefix.length() &&
												currentFrameFileName.substr(
													currentFrameFileName.length() - 4) == ".png") {

												std::string frameIndexAndExt =
													currentFrameFileName.substr(
														oldFramePrefix.length());
												std::string newFrameFileName =
													newFramePrefix + frameIndexAndExt;
												std::string oldFramePath = entry.path().string();
												std::string newFramePath =
													this->videoFolder + "/" + newFrameFileName;

												try {
													std::filesystem::rename(oldFramePath, newFramePath);
												}
												catch (
													const std::filesystem::filesystem_error& e_frame) {
													printf("Warning: Failed to "
														"rename frame %s to "
														"%s: %s\\n",
														oldFramePath.c_str(), newFramePath.c_str(),
														e_frame.what());
												}
											}
										}
									}
								}
								catch (const std::filesystem::filesystem_error& e_dir_iter) {
									printf("Er"
										"ro"
										"r "
										"it"
										"er"
										"at"
										"in"
										"g "
										"di"
										"re"
										"ct"
										"or"
										"y "
										"%s"
										" f"
										"or"
										" "
										"fr"
										"am"
										"e "
										"re"
										"na"
										"mi"
										"ng"
										": "
										"%s"
										"\\"
										"n",
										this->videoFolder.c_str(), e_dir_iter.what());
								}
							}
							this->folderName = newBaseName;
						}
						else {

							printf("Save operation "
								"may be incomplete "
								"due to "
								"folder rename "
								"failure.\\n");
							finalVideoPath = lastVideoPath;
						}
					}
					else {

						finalVideoPath = lastVideoPath;
					}
				}
				else {
					finalVideoPath = lastVideoPath;
				}

				std::filesystem::path finalPathObj(finalVideoPath);
				actualSavedVideoFolder = finalPathObj.parent_path().string();
				actualSavedVideoName = finalPathObj.filename().string();

				std::filesystem::path videoNamePath(actualSavedVideoName);
				if (videoNamePath.has_extension() &&
					videoNamePath.extension() == ".mp4") {
					actualSavedVideoName = videoNamePath.stem().string();
				}        videoHasBeenSaved = true;
				showSaveConfirmationDialog = false;
				nameBuffer[0] = '\0';

				// Clear recording state to close the recording menu
				// Only discard frames if frame export is disabled
				if (!myFrames.empty() && !isExportFramesEnabled) {
					discardMemoryFrames();
				}
				diskModeFrameIdx = 0;

				if (isExportFramesEnabled && isSafeFramesEnabled) {

					printf("Frames are in: %s with base "
						"name: %s\\n",
						this->videoFolder.c_str(), this->folderName.c_str());
				}
			}
		}

		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, UpdateVariables::colButtonRedActive);
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, UpdateVariables::colButtonRedActiveHover);
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, UpdateVariables::colButtonRedActivePress);
		if (ImGui::Button("Discard", ImVec2(100, 30))) {
			if (!isFunctionRecording) {
				// Use the centralized discard function for comprehensive cleanup
				discardRecording();

				// Handle video file cleanup if it exists outside the video folder
				try {
					if (std::filesystem::exists(lastVideoPath) &&
						lastVideoPath != this->videoFolder) {
						if (std::filesystem::is_regular_file(lastVideoPath)) {
							std::filesystem::remove(lastVideoPath);
							printf("Discarded video file: %s\\n", lastVideoPath.c_str());
						}
						else if (std::filesystem::is_directory(lastVideoPath)) {
							std::filesystem::remove_all(lastVideoPath);
							printf("Discarded (unexpected) directory at lastVideoPath: %s\\n",
								lastVideoPath.c_str());
						}
					}
				}
				catch (const std::filesystem::filesystem_error& e) {
					printf("Error discarding video/folder: %s\\n", e.what());
				}
			}
			showSaveConfirmationDialog = false;
			nameBuffer[0] = '\0';

			// Clear recording state to close the recording menu
			if (!myFrames.empty()) {
				discardMemoryFrames();
			}
			diskModeFrameIdx = 0;
			lastVideoPath.clear();
			this->videoFolder.clear();
			this->folderName.clear();      videoHasBeenSaved = false;
			isFunctionRecording = false;
		}
		ImGui::PopStyleColor(3);

		ImGui::PopFont();
		ImGui::End();
	}
	return isFunctionRecording;
}