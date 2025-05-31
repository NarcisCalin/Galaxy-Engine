#pragma once

#include <raylib.h>
#include <imgui.h>

// Input/Output utility functions
namespace IO {
    
    // Handle keyboard shortcuts with ImGui integration
    static inline bool handleShortcut(int key) {
        ImGuiIO& io = ImGui::GetIO();
        if (io.WantCaptureKeyboard) {
            return false; // Don't process shortcuts when ImGui wants keyboard
        }
        return IsKeyPressed(key);
    }

    // Additional IO utility functions can be added here in the future
    // For example:
    // - Mouse handling utilities
    // - File IO helpers
    // - Keyboard state management
    // - Input validation functions
}
