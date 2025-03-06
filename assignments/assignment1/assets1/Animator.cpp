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
		animation.setKeysAtTime(playbackTime);
		
		//interpolate between animation frames
		ew::Transform* model = animation.getModel();
		model->position = animation.getNextPos().getValue();
		
	}

	void Animator::calculatePlaybackTime(float maxDuration, float deltaTime)
	{
		deltaTime = deltaTime * playbackSpeed;
		float tempTime = playbackTime + deltaTime;
		if (tempTime >= maxDuration)
		{
			if (isLooping)
			{
				tempTime = 0 + (maxDuration - tempTime);
				setPlayBackTime(tempTime);
				return;
			}
			setPlayBackTime(maxDuration);
		}
		else if (tempTime < 0)
		{
			if(isLooping) 
			{
				tempTime = maxDuration + (tempTime);
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

}