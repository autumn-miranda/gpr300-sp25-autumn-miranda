#include "Skeleton.h"

namespace anm
{
	Skeleton::Skeleton() 
	{
		ew::Transform blank;
		skeletonTransform = blank;
		addRootJoint(blank);
	}

	Skeleton::Skeleton(ew::Transform modelTransform)
	{
		skeletonTransform = modelTransform;
		addRootJoint(modelTransform);
	}


	void Skeleton::setParentIndex(int thisIndex, int parentIndex)
	{
		modelSkeleton[thisIndex].parentIndex = parentIndex;

		if (parentIndex != -1) 
		{
			modelSkeleton[parentIndex].childIndex.push_back(thisIndex);
		}
	}

	void Skeleton::addRootJoint(ew::Transform modelTransform)
	{
		modelSkeleton[0].parentIndex = -1;
		modelSkeleton[0].localTransform = modelTransform;


		addJoint(0, 7, glm::vec3(0.0, 1.5, 0.0), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.5, 0.5, 0.5));
		addArm(3, 1, true);
		addArm(3, 4, false);
	}


	void Skeleton::addJoint(int parentIndex, int thisIndex, glm::vec3 pos, glm::vec3 rot, glm::vec3 scale)
	{
		this->setParentIndex(thisIndex, parentIndex);
		modelSkeleton[thisIndex].localTransform.position = pos;
		modelSkeleton[thisIndex].localTransform.rotation = rot;
		modelSkeleton[thisIndex].localTransform.scale = scale;
		modelSkeleton[thisIndex].globalTransform = globalizeTransform(modelSkeleton[parentIndex].globalTransform, modelSkeleton[thisIndex].localTransform);
	}


	void Skeleton::addArm(int segments, int start, bool onLeft)
	{
		for (int i = start; i < segments + start; i++) 
		{
			if (i == start) 
			{
				addJoint(0, i, glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.5, 0.5, 0.5));
			}
			else 
			{
				addJoint(i - 1, i, glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.75, 0.75, 0.75));
			}


			if (onLeft) 
			{
				int pI = (i == start) ? 0 : i - 1;
				modelSkeleton[i].localTransform.position.x *= -1.0f;
				modelSkeleton[i].globalTransform = globalizeTransform(modelSkeleton[pI].globalTransform, modelSkeleton[i].localTransform);
			}
		}
	}

	ew::Transform Skeleton::globalizeTransform(ew::Transform gt, ew::Transform lt)
	{
		ew::Transform temp;

		temp.position = gt.position + lt.position;
		temp.rotation = gt.rotation + lt.rotation;
		temp.scale = (gt.scale * lt.scale);


		return temp;
	}

	void Skeleton::calcGlobalTransforms(int start)
	{
		if (modelSkeleton[start].parentIndex <= -1)
		{
			modelSkeleton[start].globalTransform = modelSkeleton[start].localTransform;
			skeletonTransform = modelSkeleton[start].globalTransform;
		}
		else 
		{
			int parent = modelSkeleton[start].parentIndex;
			modelSkeleton[start].globalTransform = globalizeTransform(modelSkeleton[parent].globalTransform, modelSkeleton[start].localTransform);
		}
		for (int index : modelSkeleton[start].childIndex) 
		{
			calcGlobalTransforms(index);			
		}
	}


}