#pragma once
#include "Vec3Key.h"

namespace anm
{
	float KeyFrame::selectFunction(int i, float a, float b, float playback)
	{
		switch(i)
		{
		case 0:
			return findT(a, b, playback);
		case 1:
			return easeInBack(a, b, playback);
		case 2: 
			return easeInElastic(a, b, playback);
		case 3:
			return easeOutBack(a, b, playback);
		case 4:
			return easeOutCubic(a, b, playback);
		default:
			return findT(a, b, playback);
		}
	}

	float KeyFrame::findT(float a, float b, float playback)
	{
		//assert(b >= a);
		if (b == a || playback == a)
		{
			return 0.0f;
		}
		else if (b < playback) {
			return 1.0f - ((playback - b) / (playback - a));
		}
		else
		{
			return 1.0f - ((b - playback) / (b - a));
		}

	}

	float KeyFrame::easeInBack(float a, float b, float playback)
	{
		float x = abs(playback - a) / abs(b - a);
		constexpr float c1 = 1.70158;
		constexpr float c3 = c1 + 1;

		 return c3 * x * x * x - c1 * x * x;
	}

	float KeyFrame::easeOutBack(float a, float b, float playback)
	{
		if (b == a) return 0.f;

		float x = abs(playback - a) / abs(b - a);
		constexpr float c1 = 1.70158;
		constexpr float c3 = c1 + 1;

		return 1 + c3 * pow(x - 1, 3) + c1 * pow(x - 1, 2);
	}

	float KeyFrame::easeOutCubic(float a, float b, float playback)
	{
		if (b == a) return 0.f;
		float x = abs(playback - a) / abs(b - a);
		return 1 - pow(1 - x, 3);
	};

	float KeyFrame::easeInElastic(float a, float b, float playback)
	{
		constexpr float c4 = (2 * glm::pi<float>()) / 3;

		if (a == playback)
			return 0.f;

		float x = abs(playback - a) / abs(b - a);
		if (x == 0)
			return 0.f;
		if (x == 1)
			return 1.0f;
		return -pow(2, 10 * x - 10) * sin((x * 10 - 10.75) * c4);
	}

}