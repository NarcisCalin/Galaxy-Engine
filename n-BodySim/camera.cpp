#include "camera.h"
#include <iostream>
#include <algorithm>

SceneCamera::SceneCamera(){
	camera.offset = { 0.0f };
	camera.target = { 0.0f };
	camera.rotation = 0.0f;
	camera.zoom = 1.0f;
	mouseWorldPos = { 0.0f, 0.0f };
}

Camera2D SceneCamera::cameraLogic(){

	if (IsMouseButtonDown(1))
	{
		Vector2 delta = GetMouseDelta();
		delta = Vector2Scale(delta, -1.0f / camera.zoom);
		camera.target = Vector2Add(camera.target, delta);
	}

    float wheel = GetMouseWheelMove();
    if (wheel != 0 && !IsKeyDown(KEY_LEFT_CONTROL)){
        mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);

        camera.offset = GetMousePosition();

        camera.target = mouseWorldPos;

        float scale = 0.2f * wheel;
        camera.zoom = Clamp(expf(logf(camera.zoom) + scale), 0.95f, 64.0f);
    }	
	
	// RESET CAMERA
	if (IsKeyPressed(KEY_F)) {
		camera.zoom = 1.0f;
		camera.target = { 0.0f };
		camera.offset = { 0.0f };
	}
	return camera;
}
