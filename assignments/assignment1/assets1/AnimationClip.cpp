#pragma once
#include "AnimationClip.h"
#include <algorithm>

namespace anm
{

	void AnimationClip::setKeysAtTime(float time)
	{
		//std::find(fArray.begin(), fArray.end(), time) != fArray.end();
		findFramePos(time);
		findFrameRot(time);
		findFrameScale(time);
	}

	void AnimationClip::addKeyFrame(KeyFrame theFrame, std::vector<KeyFrame>& fArray)
	{
		if (theFrame.getTime() < 0) 
		{
			theFrame.setTime(0);
		}
		else if (theFrame.getTime() > maxDuration) 
		{
			theFrame.setTime(maxDuration);
		}

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

	void AnimationClip::addKeyFrame(std::vector<KeyFrame>& fArray)
	{
		fArray.push_back(KeyFrame());
	}

	void AnimationClip::addKeyFrame(std::vector<KeyFrame>& fArray, float time, glm::vec3 val)
	{
		fArray.push_back(KeyFrame(time, val));
	}

	void AnimationClip::removeKeyFrame(std::vector<KeyFrame>& fArray)
	{
		//add the frame to the passed array
		if (fArray.empty())
		{
			return;
		}

		fArray.pop_back();
	}

	void AnimationClip::editFrame(std::vector<KeyFrame>& fArray, int index, float time, glm::vec3 val)
	{
		if (fArray.size() > 0) 
		{
			fArray[index].setTime(time);
			fArray[index].setValue(val);

			std::sort(fArray.begin(), fArray.end(), [](KeyFrame a, KeyFrame b)
				{
					return a.getTime() < b.getTime();
				});
		}
	}

	void AnimationClip::sortArray(std::vector<KeyFrame>& fArray)
	{
		if (fArray.size() > 1)
		{
			std::sort(fArray.begin(), fArray.end(), [](KeyFrame a, KeyFrame b)
				{
					return a.getTime() < b.getTime();
				});
		}
	}


	void AnimationClip::findFramePos(float time)
	{
		if (positionKeys.size() > 0) {
			for (int i = 0; i < positionKeys.size(); i++)
			{
				if (positionKeys[i].getTime() > time)
				{
					nextFrame[0] = positionKeys[i];
					if (i > 0)
					{
						prevFrame[0] = positionKeys[i - 1];
					}
					else
					{
						prevFrame[0] = KeyFrame(time - 0.1f, model->position);
					}
					return;
				}
			}

			//if nextFrame is outside the array, just take the last element
			nextFrame[0] = positionKeys[positionKeys.size() - 1];
			nextFrame[0].setTime(maxDuration);
			prevFrame[0] = positionKeys[positionKeys.size() - 1];
		}
		else 
		{
			nextFrame[0] = KeyFrame(maxDuration, modelDefaults.position);
			prevFrame[0] = KeyFrame(0.0f, modelDefaults.position);
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
						prevFrame[1] = KeyFrame(time - 0.1f, mRotation);
					}
					return;
				}
			}

			//if nextFrame is outside the array, just take the last element
			nextFrame[1] = rotationKeys[rotationKeys.size() - 1];
			nextFrame[1].setTime(maxDuration);
			prevFrame[1] = KeyFrame(time, mRotation);
		}
		else
		{
			nextFrame[1] = KeyFrame(maxDuration, glm::degrees(glm::eulerAngles(modelDefaults.rotation)));
			prevFrame[1] = KeyFrame(time - 0.1f, glm::degrees(glm::eulerAngles(modelDefaults.rotation)));
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
						prevFrame[2] = KeyFrame(time - 0.1f, model->scale);
					}
					return;
				}
			}

			//if nextFrame is outside the array, just take the last element
			nextFrame[2] = scaleKeys[scaleKeys.size() - 1];
			nextFrame[2].setTime(maxDuration);
			prevFrame[2] = KeyFrame(time, model->scale);
		}
		else
		{
			nextFrame[2] = KeyFrame(maxDuration, modelDefaults.scale);
			prevFrame[2] = KeyFrame(time - 0.1f, model->scale);
		}
	}

}	