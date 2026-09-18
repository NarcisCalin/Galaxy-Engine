#pragma once

#include "parameters.h"

struct GeneralCell {
	glm::vec2 pos;
	float size;
	int x;
	int y;
	Color color = BLACK;
	bool isOn;

	GeneralCell(glm::vec2 pos, float size, int x, int y, bool isOn)
		: pos(pos), size(size), x(x), y(y), isOn(isOn) {
	}

	void draw() {

		if (isOn) {
			color = WHITE;
		}
		else {
			color = BLACK;
		}

		DrawRectangleV({ pos.x, pos.y }, { size, size }, color);
	}
};

struct Grid {
	std::vector<GeneralCell> cells;

	int res = 256;

	glm::vec2 screenSize = { 1920.0f, 1080.0f };

	float ratio = screenSize.y / screenSize.x;

	int resX = res;
	int resY = float(res) * ratio;

	bool initGridFlag = true;

	float cellSize = screenSize.x / float(res) * 2.0f;

	void initGrid() {

		if (screenSize.x != float(GetScreenWidth()) || screenSize.y != float(GetScreenHeight())) {
			initGridFlag = true;

			screenSize.x = float(GetScreenWidth());
			screenSize.y = float(GetScreenHeight());
		}

		if (initGridFlag) {

			cells.clear();

			ratio = screenSize.y / screenSize.x;

			resX = res;
			resY = float(res) * ratio;

			cellSize = (screenSize.x / float(res)) * 2.0f;

			if (screenSize.y > screenSize.x) {
				ratio = screenSize.x / screenSize.y;

				resX = float(res) * ratio;
				resY = res;

				cellSize = (screenSize.y / float(res)) * 2.0f;
			}

			for (int y = 0; y < resY; y++) {
				for (int x = 0; x < resX; x++) {
					cells.emplace_back(glm::vec2{ x * cellSize, y * cellSize }, cellSize, x, y, false);
				}
			}

			initGridFlag = false;
		}
	}

	void gameOfLife() {

		std::vector<GeneralCell> copyCells = cells;

		for (int i = 0; i < cells.size(); i++) {

			GeneralCell& c = cells[i];

			int index = c.y * resX + c.x;

			int neighbors = 0;

			for (int oy = -1; oy <= 1; oy++) {
				for (int ox = -1; ox <= 1; ox++) {

					if (ox == 0 && oy == 0)
						continue;

					int nx = c.x + ox;
					int ny = c.y + oy;

					if (nx >= 0 && nx < resX && ny >= 0 && ny < resY) {
						int neighborIndex = ny * resX + nx;

						GeneralCell& nc = copyCells[neighborIndex];

						if (nc.isOn) {
							neighbors++;
						}
					}
				}
			}

			if (c.isOn && neighbors < 2) {
				c.isOn = false;
			}

			if (c.isOn && neighbors > 3) {
				c.isOn = false;
			}

			if (!c.isOn && neighbors == 3) {
				c.isOn = true;
			}
		}
	}

	void fallingSand() {

		for (int i = cells.size() - 1; i >= 0; i--) {

			GeneralCell& c = cells[i];

			int index = c.y * resX + c.x;

			int b = (c.y + 1) * resX + c.x;
			int br = (c.y + 1) * resX + (c.x + 1);
			int bl = (c.y + 1) * resX + (c.x - 1);

			if (c.isOn && !cells[b].isOn) {
				if (c.y != resY - 1) {
					c.isOn = false;
					cells[b].isOn = true;
				}
			}
			else if (c.isOn && cells[b].isOn) {

				int rand = GetRandomValue(0, 1);

				if (rand == 0) {

					if (c.x != 0) {
						if (!cells[bl].isOn) {
							c.isOn = false;
							cells[bl].isOn = true;
						}
					}
				}
				else {
					if (c.x != resX - 1) {
						if (!cells[br].isOn) {
							c.isOn = false;
							cells[br].isOn = true;
						}
					}
				}

			}
		}
	}

	void gridLogic(UpdateVariables& myVar, float& brushRadius) {
		for (int i = 0; i < cells.size(); i++) {

			GeneralCell& c = cells[i];

			if (IO::mouseDown(0) && !IO::shortcutDown(KEY_X)) {

				float dist = glm::distance(c.pos, myVar.mouseWorldPos);

				if (dist < brushRadius) {
					c.isOn = true;
				}
			}
			else if (IO::mouseDown(0) && IO::shortcutDown(KEY_X)) {
				float dist = glm::distance(c.pos, myVar.mouseWorldPos);

				if (dist < brushRadius) {
					c.isOn = false;
				}
			}
		}

		/*if (myVar.timeFactor != 0.0f && !IO::mouseDown(0)) {
			gameOfLife();
		}*/

		if (myVar.timeFactor != 0.0f) {
			fallingSand();
		}

	}

	void drawCells() {

		float minX = cells[0].pos.x;
		float minY = cells[0].pos.y;
		float maxX = cells[0].pos.x;
		float maxY = cells[0].pos.y;

		for (size_t i = 1; i < cells.size(); i++) {
			GeneralCell& c = cells[i];

			if (c.pos.x < minX) minX = c.pos.x;
			if (c.pos.y < minY) minY = c.pos.y;

			if (c.pos.x > maxX) maxX = c.pos.x;
			if (c.pos.y > maxY) maxY = c.pos.y;
		}

		float padding = 20.0f;

		DrawRectangleV(
			{ minX - padding, minY - padding },
			{ (maxX - minX) + padding * 2, (maxY - minY) + padding * 2 },
			DARKGRAY
		);

		for (size_t i = 0; i < cells.size(); i++) {
			GeneralCell& c = cells[i];
			c.draw();
		}
	}

};
