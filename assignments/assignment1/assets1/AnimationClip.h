#pragma once
/*Holds the data for an animation. For keyframe lists, we assume keyframe times are in ascending order.*/
namespace anm 
{
	class AnimationClip {
	public:
		float duration;
		Vec3Key[] positionKeys;
		Vec3Key[] rotationKeys;//store as Euler angles
		Vec3Key[] scaleKeys;

		Vec3Key getKeyAtTime();
		void createKey();
	private:
	};


}