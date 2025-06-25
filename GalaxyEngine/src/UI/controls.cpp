#include "UI/controls.h"
#include "UI/UI.h"
#include "parameters.h"

void Controls::showControls() {
    if (isShowControlsEnabled) {

        float screenW = static_cast<float>(GetScreenWidth());
        float screenH = static_cast<float>(GetScreenHeight());

        ImVec2 controlSize = { screenW * 0.15f, screenH * 0.4f };

        controlSize.x = std::clamp(controlSize.x, 400.0f, 2000.0f);
        controlSize.y = std::clamp(controlSize.y, 600.0f, 2000.0f);

        ImGui::SetNextWindowSize(controlSize, ImGuiCond_Appearing);
        ImGui::SetNextWindowPos(ImVec2(screenW * 0.5f - controlSize.x * 0.5f, screenH * 0.5f - controlSize.y * 0.5f), ImGuiCond_Appearing);

        ImGui::Begin("Controls", nullptr, ImGuiWindowFlags_NoCollapse);

        for (size_t i = 0; i < controlsArray.size(); i++) {

            std::string text = controlsArray[i];

           /* float windowWidth = ImGui::GetWindowSize().x;
            float textWidth = ImGui::CalcTextSize(text.c_str()).x;

            ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);*/

            ImGui::Text("%s", text.c_str());
        }

        ImGui::End();
    }
}

void Controls::showInfo(bool& fullscreen) {
    if (isInformationEnabled) {

        float screenW = static_cast<float>(GetScreenWidth());
        float screenH = static_cast<float>(GetScreenHeight());

        ImVec2 infoSize = { screenW * 0.3f, screenH * 0.4f };

        infoSize.x = std::clamp(infoSize.x, 700.0f, 2000.0f);
        infoSize.y = std::clamp(infoSize.y, 600.0f, 2000.0f);

        ImGui::SetNextWindowSize(infoSize, ImGuiCond_Appearing);
        ImGui::SetNextWindowPos(ImVec2(screenW * 0.5f - infoSize.x * 0.5f, screenH * 0.5f - infoSize.y * 0.5f), ImGuiCond_Appearing);

        ImGui::Begin("Information", nullptr, ImGuiWindowFlags_NoCollapse);

        for (size_t i = 0; i < infoArray.size(); i++) {

            std::string text = infoArray[i];

            /* float windowWidth = ImGui::GetWindowSize().x;
             float textWidth = ImGui::CalcTextSize(text.c_str()).x;

             ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);*/

            ImGui::Text("%s", text.c_str());
        }

        ImVec2 linkButtonSize= { 200.0f, 30.0f };

        ImGui::TextColored(ImVec4(0.8f, 0.0f, 0.0f, 1.0f), "The links might not work on Linux");

        bool placeHolder = false;

        bool enabled = true;

        if (UI::buttonHelper("GitHub Repository", "Go to GitHub", placeHolder, linkButtonSize.x, linkButtonSize.y, enabled, enabled)) {
            fullscreen = false;
            OpenURL("https://github.com/NarcisCalin/Galaxy-Engine");
        }

        ImGui::SameLine();

        if (UI::buttonHelper("Join Our Discord Community!", "Click to join!", placeHolder, linkButtonSize.x, linkButtonSize.y, enabled, enabled)) {
            fullscreen = false;
            OpenURL("https://discord.gg/Xd5JUqNFPM");
        }

        ImGui::SameLine();

        if (UI::buttonHelper("Soundtrack by HAVA", "Check their work!", placeHolder, linkButtonSize.x, linkButtonSize.y, enabled, enabled)) {
            fullscreen = false;
            OpenURL("https://soundcloud.com/user-150017147");
        }

        ImGui::End();
    }
}
