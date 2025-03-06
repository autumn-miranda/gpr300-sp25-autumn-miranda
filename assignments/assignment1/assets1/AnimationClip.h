#pragma once
#include "Vec3Key.h"
#include <vector>

#include "ew/transform.h"
/*Holds the data for an animation. For keyframe lists, we assume keyframe times are in ascending order.*/
namespace anm 
{
	class AnimationClip {
	public:
		float maxDuration;
		std::vector<KeyFrame> positionKeys;
		std::vector<KeyFrame> rotationKeys;//store as Euler angles
		std::vector<KeyFrame> scaleKeys;

		KeyFrame nextFrame[3];
		KeyFrame prevFrame[3];
		ew::Transform* model;

		void setKeysAtTime(float time);
		void addKeyFrame(KeyFrame& theFrame, std::vector<KeyFrame> fArray);

		void removeKeyFrame(std::vector<KeyFrame> fArray);

		void findFramePos(float time);
		void findFrameRot(float time);
		void findFrameScale(float time);
		KeyFrame getNextPos() { return nextFrame[0]; };
		KeyFrame getPrevPos() { return prevFrame[0]; };
		KeyFrame getNextRot() { return nextFrame[1]; };
		KeyFrame getPrevRot() { return prevFrame[1]; };
		KeyFrame getNextScale() { return nextFrame[2]; };
		KeyFrame getPrevScale() { return prevFrame[2]; };

		void setModel(ew::Transform* transform) { model = transform; };

		float getDuration() { return maxDuration; };
		std::vector<KeyFrame> getPosArray() { return positionKeys; };
		std::vector<KeyFrame> getRotArray() { return rotationKeys; };
		std::vector<KeyFrame> getScaleArray() { return scaleKeys; };


	private:
	};


}