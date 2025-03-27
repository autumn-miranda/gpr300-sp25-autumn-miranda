#include <vector>
#include "ew/transform.h"
#include <string>

namespace anm
{
	class Skeleton
	{
	public:
		struct Joint
		{
			int parentIndex = -1;
			std::vector<int> childIndex;

			ew::Transform localTransform;
			ew::Transform globalTransform;

			ew::Transform getGlobalTransform() { return globalTransform; };
		};

		ew::Transform skeletonTransform = modelSkeleton[0].globalTransform;
		Joint modelSkeleton[8];
		std::string jointNames[8] = {"Torso", "Left Shoulder", "Left Elbow", "Left Wrist", "Right Shoulder", "Right Elbow", "Right Wrist", "Head"};

		Skeleton();
		Skeleton(ew::Transform modelTransform);

		glm::mat4 getLocalTransform(Joint joint)
		{
			return joint.localTransform.modelMatrix();
		};
		glm::mat4 getGlobalTransform(Joint joint)
		{
			return joint.globalTransform.modelMatrix();
		};
		glm::mat4 getSkeletonTransform()
		{
			return skeletonTransform.modelMatrix();
		};

		void setParentIndex(int thisIndex, int parentIndex);

		void addRootJoint(ew::Transform modelTransform);
		void addJoint(int parentIndex, int thisIndex, glm::vec3 pos, glm::vec3 rot, glm::vec3 scale);
		void addArm(int segments, int start, bool onLeft);

		ew::Transform globalizeTransform(ew::Transform gt, ew::Transform lt);
		void calcGlobalTransforms(int start);
	private:
	};

}