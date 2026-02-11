#pragma once

#include "Particles/particle.h"

struct RayMarcher {

	struct Pixel {
		int x;
		int y;
		int size;
		Color color;
	};

	int screenResX = 1920;
	int screenResY = 1080;

	int pixelSize = 10;

	std::vector<Pixel> pixels;

	bool initPixels = true;

	void initPixelGrid() {

		if (initPixels) {
			int pixelAmountX = screenResX / pixelSize;
			int pixelAmountY = screenResY / pixelSize;

			for (int y = 0; y < pixelAmountY; y++) {
				for (int x = 0; x < pixelAmountX; x++) {

					pixels.emplace_back(Pixel{ x * pixelSize, y * pixelSize, pixelSize , Color{ 255, 0, 0, 255 } });
				}
			}

			initPixels = false;
		}
	}

	void drawPixels() {
		for (Pixel& p : pixels) {

			DrawRectangle(p.x, p.y, p.size, p.size, { p.color.r,p.color.g, p.color.b, p.color.a });
		}
	}
};