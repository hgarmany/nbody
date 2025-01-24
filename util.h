#pragma once

const float pi = 3.141592653589793f;
const int WIDTH = 900, HEIGHT = 600;
const float MAX_FRAME_TIME = 1.0f / 60.0f;

enum render_modes : uint8_t
{
	MODE_TEX,
	MODE_SOLID,
	MODE_CUBEMAP
};