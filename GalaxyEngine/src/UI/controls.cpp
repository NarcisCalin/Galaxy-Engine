#include "../../include/UI/controls.h"
#include "../../include/parameters.h"

void Controls::showControls(UpdateVariables& myVar) {

	Button extendedControlsButton = { Vector2{ 20.0f, 140.0f }, Vector2{ 200.0f, 30.0f }, "Extended Controls", true };

	if (isShowControlsEnabled) {
		for (size_t i = 0; i < controlsArray.size(); i++) {
			DrawText(TextFormat("%s", controlsArray[i].c_str()), 25, 20 + 20 * static_cast<int>(i), 15, WHITE);
		}
		bool buttonExtendedControlsHovering = extendedControlsButton.buttonLogic(isShowExtendedControlsEnabled, myVar);

		if (buttonExtendedControlsHovering) {
			myVar.isMouseNotHoveringUI = false;
			myVar.isDragging = false;
		}

		if (isShowExtendedControlsEnabled) {
			Vector2 pageButtonPos = {
				static_cast<float>(myVar.screenWidth) / 2.0f,
				static_cast<float>(myVar.screenHeight) / 2.0f - controlsBoxSizeY / 2.0f - 14.0f };
			Button changePageButton(pageButtonPos,
				{ 14.0f, 14.0f }, "", false);

			bool buttonNextPageHovering = changePageButton.buttonLogic(nextPage, myVar);

			if (buttonNextPageHovering) {
				myVar.isMouseNotHoveringUI = false;
				myVar.isDragging = false;
			}

			DrawTriangle(
				{ pageButtonPos.x + 3.0f, pageButtonPos.y + 5.0f },
				{ pageButtonPos.x + 7.0f, pageButtonPos.y + 11.0f },
				{ pageButtonPos.x + 11.0f ,pageButtonPos.y + 5.0f }, WHITE);

			DrawRectangle(myVar.screenWidth / 2 - controlsBoxSizeX / 2,
				myVar.screenHeight / 2 - controlsBoxSizeY / 2,
				controlsBoxSizeX, controlsBoxSizeY,
				{ 170,170,170,170 });

			if (!nextPage) {
				DrawText("PAGE 1.", myVar.screenWidth / 2 - controlsBoxSizeX / 2 + 20, myVar.screenHeight / 2 - controlsBoxSizeY / 2 + 20, 30, WHITE);
				for (size_t i = 0; i < extendedControlsArrayParticles.size(); i++) {
					DrawText(TextFormat("%s", extendedControlsArrayParticles[i].c_str()),
						(myVar.screenWidth / 2 - controlsBoxSizeX / 2) + 20,
						((myVar.screenHeight / 2 - controlsBoxSizeY / 2) + 70) + 30 * static_cast<int>(i), 18, WHITE);
				}

				for (size_t i = 0; i < extendedControlsArrayCamAndSelec.size(); i++) {
					DrawText(TextFormat("%s", extendedControlsArrayCamAndSelec[i].c_str()),
						(myVar.screenWidth / 2 - controlsBoxSizeX / 2) + 20,
						((myVar.screenHeight / 2 - controlsBoxSizeY / 2) + 340) + 30 * static_cast<int>(i), 18, WHITE);
				}
			}
			else {
				DrawText("PAGE 2.", myVar.screenWidth / 2 - controlsBoxSizeX / 2 + 20, myVar.screenHeight / 2 - controlsBoxSizeY / 2 + 20, 30, WHITE);
				for (size_t i = 0; i < extendedControlsArrayUtility.size(); i++) {
					DrawText(TextFormat("%s", extendedControlsArrayUtility[i].c_str()),
						(myVar.screenWidth / 2 - controlsBoxSizeX / 2) + 20,
						((myVar.screenHeight / 2 - controlsBoxSizeY / 2) + 70) + 30 * static_cast<int>(i), 18, WHITE);
				}
			}

		}
	}
}

void Controls::showMoreInfo(UpdateVariables& myVar){

	if (myVar.isInformationEnabled) {
		DrawRectangle(myVar.screenWidth / 2 - controlsBoxSizeX / 2,
			myVar.screenHeight / 2 - controlsBoxSizeY / 2,
			controlsBoxSizeX + 175, controlsBoxSizeY,
			{ 170,170,170,170 });

		for (size_t i = 0; i < informationArray.size(); i++) {
			DrawText(TextFormat("%s", informationArray[i].c_str()),
				(myVar.screenWidth / 2 - controlsBoxSizeX / 2) + 20,
				((myVar.screenHeight / 2 - controlsBoxSizeY / 2) + 70) + 30 * static_cast<int>(i), 18, WHITE);
		}
	}
}
