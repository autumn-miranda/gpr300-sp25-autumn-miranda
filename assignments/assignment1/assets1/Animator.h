#pragma once
#include "AnimationClip.h"
//Responsible for playing back an animation.
namespace anm 
{
	class Animator {
	public: 
		AnimationClip* clip;
		bool isPlaying = false;
		float playbackSpeed = 1.0;
		bool isLooping = false;
		float playbackTime = 0.0;


		void playAnimation(AnimationClip& animation, float deltaTime);
		void calculatePlaybackTime(float maxDuration, float deltaTime);

		void setPlayBackTime(float time) { playbackTime = (time >= 0)? time: 0.f; };
		void setPlayBackSpeed(float speed) { playbackSpeed = speed; };
		void setPlaying(bool play) { isPlaying = play; };
		void setLooping(bool loop) { isLooping = loop; };

		glm::vec3 interpolatePos(KeyFrame& a, KeyFrame& b);
		glm::vec3 interpolateRot(KeyFrame& a, KeyFrame& b);
		glm::vec3 interpolateScale(KeyFrame& a, KeyFrame& b);
		glm::vec3 lerp(glm::vec3 a, glm::vec3 b, float t, bool isAngle);
		float findT(KeyFrame& a, KeyFrame& b);
	private: 
	};
	// :) + :| + :(  
}