#pragma once
#include "AnimationClip.h"

namespace anm
{

	void AnimationClip::setKeysAtTime(float time)
	{
		//std::find(fArray.begin(), fArray.end(), time) != fArray.end();
		findFramePos(time);
		findFrameRot(time);
		findFrameScale(time);
	}

	void AnimationClip::addKeyFrame(KeyFrame& theFrame, std::vector<KeyFrame> fArray)
	{
		//add the frame to the passed array
		if (fArray.empty()) 
		{
			fArray.push_back(theFrame);
			return;
		}

		for(int i = 0; i < fArray.size(); i++) 
		{
			if (fArray[i].getTime() > theFrame.getTime()) 
			{
				fArray.insert(fArray.begin() + i, theFrame);
				return;
			}
		}

		fArray.push_back(theFrame);
	}

	void AnimationClip::removeKeyFrame(std::vector<KeyFrame> fArray)
	{
		//add the frame to the passed array
		if (fArray.empty())
		{
			return;
		}

		fArray.pop_back();
	}


	void AnimationClip::findFramePos(float time)
	{
		if (positionKeys.size() > 0) {
			for (int i = 0; i < positionKeys.size(); i++)
			{
				if (positionKeys[i].getTime() >= time)
				{
					nextFrame[0] = positionKeys[i];
					if (i > 0)
					{
						prevFrame[0] = positionKeys[i - 1];
					}
					else
					{
						prevFrame[0] = KeyFrame(time, model->position);
					}
					return;
				}
			}

			//if nextFrame is outside the array, just take the last element
			nextFrame[0] = positionKeys[positionKeys.size() - 1];
			prevFrame[0] = KeyFrame(time, model->position);
		}
		else 
		{
			nextFrame[0] = KeyFrame(maxDuration, model->position);
			prevFrame[0] = KeyFrame(time, model->position);
		}
	}
	void AnimationClip::findFrameRot(float time)
	{
		glm::vec3 mRotation = glm::degrees(glm::eulerAngles(model->rotation));
		if (rotationKeys.size() > 0) {
			for (int i = 0; i < rotationKeys.size(); i++)
			{
				if (rotationKeys[i].getTime() >= time)
				{
					nextFrame[1] = rotationKeys[i];
					if (i > 0)
					{
						prevFrame[1] = rotationKeys[i - 1];
					}
					else
					{
						prevFrame[1] = KeyFrame(time, mRotation);
					}
					return;
				}
			}

			//if nextFrame is outside the array, just take the last element
			nextFrame[1] = rotationKeys[rotationKeys.size() - 1];
			prevFrame[1] = KeyFrame(time, mRotation);
		}
		else
		{
			nextFrame[1] = KeyFrame(maxDuration, mRotation);
			prevFrame[1] = KeyFrame(time, mRotation);
		}
	}
	void AnimationClip::findFrameScale(float time)
	{
		if (scaleKeys.size() > 0) {
			for (int i = 0; i < scaleKeys.size(); i++)
			{
				if (scaleKeys[i].getTime() >= time)
				{
					nextFrame[2] = scaleKeys[i];
					if (i > 0)
					{
						prevFrame[2] = scaleKeys[i - 1];
					}
					else
					{
						prevFrame[2] = KeyFrame(time, model->scale);
					}
					return;
				}
			}

			//if nextFrame is outside the array, just take the last element
			nextFrame[2] = scaleKeys[scaleKeys.size() - 1];
			prevFrame[2] = KeyFrame(time, model->scale);
		}
		else
		{
			nextFrame[2] = KeyFrame(maxDuration, model->scale);
			prevFrame[2] = KeyFrame(time, model->scale);
		}
	}


}	