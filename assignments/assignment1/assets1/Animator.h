#pragma once
//Responsible for playing back an animation.
using namespace anm 
{
	class Animator {
	public: 
		AnimationClip* clip;
		bool isPlaying;
		float playBackSpeed;
		bool isLooping;
		float playBackTime;

		void playAnimation();

		void setPlayBackTime(float time) { playBackTime = time; };
		void setPlaying(bool play) { isPlaying = play; };
		void setPlaying(bool loop) { isLooping = loop; };
	private: 
	};

}