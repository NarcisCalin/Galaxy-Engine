#pragma once
#include <iostream>

#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"


class SceneCamera {
public:
	Camera2D camera;

	Vector2 mouseWorldPos;

	SceneCamera();

	Camera2D cameraLogic();
};