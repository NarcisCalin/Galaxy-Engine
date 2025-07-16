#pragma once

// Input/Output utility functions
namespace IO {
    
    // Handle keyboard shortcuts with ImGui integration
    static inline bool shortcutPress(int key) {
        ImGuiIO& io = ImGui::GetIO();
        if (io.WantCaptureKeyboard) {
            return false;
        }
        return IsKeyPressed(key);
    }

    static inline bool shortcutDown(int key) {
        ImGuiIO& io = ImGui::GetIO();
        if (io.WantCaptureKeyboard) {
            return false;
        }
        return IsKeyDown(key);
    }

    static inline bool shortcutReleased(int key) {
        ImGuiIO& io = ImGui::GetIO();
        if (io.WantCaptureKeyboard) {
            return false;
        }
        return IsKeyReleased(key);
    }

    static inline bool mousePress(int key) {
        ImGuiIO& io = ImGui::GetIO();
        if (io.WantCaptureMouse) {
            return false;
        }
        return IsMouseButtonPressed(key);
    }

    static inline bool mouseDown(int key) {
        ImGuiIO& io = ImGui::GetIO();
        if (io.WantCaptureMouse) {
            return false;
        }
        return IsMouseButtonDown(key);
    }

    static inline bool mouseReleased(int key) {
        ImGuiIO& io = ImGui::GetIO();
        if (io.WantCaptureMouse) {
            return false;
        }
        return IsMouseButtonReleased(key);
    }

    // Additional IO utility functions can be added here in the future
    // For example:
    // - Mouse handling utilities
    // - File IO helpers
    // - Keyboard state management
    // - Input validation functions
}
