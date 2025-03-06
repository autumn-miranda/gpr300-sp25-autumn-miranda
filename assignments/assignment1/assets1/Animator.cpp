#pragma once
#include "Animator.h"

namespace anm
{
	void Animator::playAnimation(AnimationClip& animation, float deltaTime)
	{
		if (!isPlaying)
			return;

		calculatePlaybackTime(animation.getDuration(), deltaTime);
		animation.setKeysAtTime(playbackTime);
		
		//interpolate between animation frames
		

		
	}

	void Animator::calculatePlaybackTime(float maxDuration, float deltaTime)
	{
		deltaTime = deltaTime * playbackSpeed;
		if (playbackTime + deltaTime > maxDuration)
		{
			if (isLooping)
			{
				int temp = playbackTime / maxDuration;
				float tempTime = playbackTime - (temp * maxDuration);
				setPlayBackTime(tempTime);
				return;
			}
			setPlayBackTime(maxDuration);
		}
		else if (playbackTime + deltaTime < 0)
		{
			if(isLooping) 
			{
				float tempTime = maxDuration + (playbackTime + deltaTime);
				setPlayBackTime(tempTime);
				return;
			}

			setPlayBackTime(0.f);
		}
		else 
		{
			setPlayBackTime(playbackTime + deltaTime);
		}

	}

}