#pragma once
#include <glm/vec3.hpp> // glm::vec3  https://glm.g-truc.net/0.9.8/index.html 
#include <glm/gtc/quaternion.hpp>
/*Holds a single vec3 keyframe in an AnimationClip.*/

namespace anm 
{
	class KeyFrame {
	public: 

		float time;
		glm::vec3 value;


		KeyFrame()
		{
			time = 0;
			value = glm::vec3(0.0, 0.0, 0.0);
		}
		KeyFrame(float t, glm::vec3 val)
		{
			setTime(t);
			value = val;
		}

		void setValue(glm::vec3 newValue) 
		{
			value = newValue;
		}

		void setX(float x)
		{
			value.x = x;
		}

		void setY(float y)
		{
			value.y = y;
		}

		void setZ(float z)
		{
			value.z = z;
		}

		glm::vec3 getValue() 
		{
			return value;
		};

		void setTime(float playTime)
		{
			time = playTime;
			if (time < 0) 
			{
				time = 0.0f;
			}
		}

		float getTime()
		{
			return time;
		};

		//https://gamedev.stackexchange.com/questions/13436/glm-euler-angles-to-quaternion
		glm::quat eulerToQuat(glm::vec3 euler)
		{
			return glm::quat(glm::radians(euler));
		};

		//https://www.devgem.io/posts/converting-quaternions-to-euler-angles-in-glm
		glm::vec3 quatToEuler(glm::quat quaternion)
		{
			return glm::degrees(glm::eulerAngles(quaternion));
		};

	private: 
	};

}