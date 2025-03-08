#pragma once
#include "Animator.h"
#include "ew/transform.h"

namespace anm
{
	void Animator::playAnimation(AnimationClip& animation, float deltaTime)
	{
		if (!isPlaying)
			return;

		calculatePlaybackTime(animation.getDuration(), deltaTime);
		animation.sortArray(animation.getPosArray());
		animation.sortArray(animation.getRotArray());
		animation.sortArray(animation.getScaleArray());
		animation.setKeysAtTime(playbackTime);
		
		//interpolate between animation frames
		ew::Transform* model = animation.getModel();
		model->position = interpolatePos(animation.getPrevPos(), animation.getNextPos());

		model->scale = interpolateScale(animation.getPrevScale(), animation.getNextScale());

		model->rotation = interpolateRot(animation.getPrevRot(), animation.getNextRot());
		
	}

	void Animator::calculatePlaybackTime(float maxDuration, float deltaTime)
	{
		deltaTime = deltaTime * playbackSpeed;
		float tempTime = playbackTime + deltaTime;
		if (tempTime >= maxDuration)
		{
			if (isLooping)
			{
				tempTime = 0 + (tempTime - maxDuration);
				setPlayBackTime(tempTime);
				return;
			}
			setPlayBackTime(maxDuration);
		}
		else if (tempTime < 0)
		{
			if(isLooping) 
			{
				tempTime = maxDuration + tempTime;
				setPlayBackTime(tempTime);
				return;
			}

			setPlayBackTime(0.f);
		}
		else 
		{
			setPlayBackTime(tempTime);
		}

	}

	glm::vec3 Animator::interpolatePos(KeyFrame& a, KeyFrame& b)
	{
		float t = findT(a, b);
		
		return lerp(a.getValue(), b.getValue(), t, false);
	}
	glm::vec3 Animator::interpolateRot(KeyFrame& a, KeyFrame& b)
	{
		float t = findT(a, b);

		return lerp(a.getValue(), b.getValue(), t, true);
	}
	glm::vec3 Animator::interpolateScale(KeyFrame& a, KeyFrame& b)
	{
		float t = findT(a, b);
		return lerp(a.getValue(), b.getValue(), t, false);
	}

	glm::vec3 Animator::lerp(glm::vec3 a, glm::vec3 b, float t, bool isAngle) 
	{
		if (!isAngle) 
		{
			//return a + ((b - a) * t);
			return glm::mix(a, b, t);
		}

		glm::quat A = glm::radians(a);
		glm::quat B = glm::radians(b);

		glm::quat result = glm::slerp(A, B, t);

		return glm::eulerAngles(result);
	}


	float Animator::findT(KeyFrame& a, KeyFrame& b) 
	{
		return a.selectFunction(a.easing, a.getTime(), b.getTime(), playbackTime);
	}
}