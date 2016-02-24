#pragma once
#include "GameComponent.h"
namespace Library
{
	typedef void(*Material)();

	class PhysicsComponent :
		public GameComponent
	{
	public:
		PhysicsComponent();
		virtual ~PhysicsComponent();

		void SetPhysicsMaterial(Material m);
	protected:
		virtual void UpdateGravity();
		virtual void UpdateCollisions() = 0;
		
		static std::vector<PhysicsComponent*> allPhysicsComponents;
		static int sNextSlot;
		
		Material mMaterial;
	private:
		int mUID;
	};

}