#pragma once

// Input/Output utility functions
namespace IO {
    
    // Handle keyboard shortcuts with ImGui integration
    static inline bool shortcutPress(int key) {
        ImGuiIO& io = ImGui::GetIO();
        if (io.WantCaptureKeyboard) {
            return false; // Don't process shortcuts when ImGui wants keyboard
        }
        return IsKeyPressed(key);
    }

    static inline bool shortcutDown(int key) {
        ImGuiIO& io = ImGui::GetIO();
        if (io.WantCaptureKeyboard) {
            return false; // Don't process shortcuts when ImGui wants keyboard
        }
        return IsKeyDown(key);
    }

    static inline bool shortcutReleased(int key) {
        ImGuiIO& io = ImGui::GetIO();
        if (io.WantCaptureKeyboard) {
            return false; // Don't process shortcuts when ImGui wants keyboard
        }
        return IsKeyReleased(key);
    }

    // Additional IO utility functions can be added here in the future
    // For example:
    // - Mouse handling utilities
    // - File IO helpers
    // - Keyboard state management
    // - Input validation functions
}
