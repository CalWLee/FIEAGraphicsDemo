#include "pch.h"
#include "PhysicsComponent.h"

namespace Library
{
#define DEFAULT_SIZE 64

	PhysicsComponent::PhysicsComponent()
		: mMaterial(nullptr)
	{
		if (allPhysicsComponents.capacity() < DEFAULT_SIZE)
		{
			allPhysicsComponents.reserve(DEFAULT_SIZE);
			sNextSlot = 0;
		}
		else if (allPhysicsComponents.size() >= allPhysicsComponents.capacity())
		{
			allPhysicsComponents.reserve(allPhysicsComponents.capacity() * 2);
			sNextSlot = allPhysicsComponents.size();
		}

		mUID = sNextSlot;
		allPhysicsComponents.at(mUID) = this;
	}


	PhysicsComponent::~PhysicsComponent()
	{		
		allPhysicsComponents.at(mUID) = nullptr;
		sNextSlot = mUID;
	}
	void PhysicsComponent::SetPhysicsMaterial(Material m)
	{
		mMaterial = m;
	}
	void PhysicsComponent::UpdateGravity()
	{
	}
}