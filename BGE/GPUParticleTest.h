#pragma once
#include "Game.h"
#include "GPUParticleEffect.h"
#include "PhysicsFactory.h"

namespace BGE
{
	class GPUParticleTest
		: public Game
	{
	public:
		GPUParticleTest(void);
		~GPUParticleTest(void);
		float elapsed;
		bool Initialise();
		void Update(float timeDelta);
		std::shared_ptr<GPUParticleEffect> thrusterL;
		std::shared_ptr<GPUParticleEffect> thrusterR;
		std::shared_ptr<GPUParticleEffect> particles2;
		std::shared_ptr<GPUParticleEffect> snow;

		std::shared_ptr<GPUParticleEffect> toggleParticles;

		std::shared_ptr<GPUParticleEffect> stayAliveParticles;
		shared_ptr<GameComponent> ship1;
		shared_ptr<GameComponent> candle;		
		shared_ptr<GameComponent> hand;
		glm::quat handStartOrientation;
		glm::quat handEndOrientation;
		float handTimeForLerp;
		float handTime;
		float timecount;

		// Physics stuff
		std::shared_ptr<PhysicsFactory> physicsFactory;
		btDiscreteDynamicsWorld * dynamicsWorld;
				btBroadphaseInterface* broadphase;
 
		// Set up the collision configuration and dispatcher
		btDefaultCollisionConfiguration * collisionConfiguration;
		btCollisionDispatcher * dispatcher;
 
		// The actual physics solver
		btSequentialImpulseConstraintSolver * solver;


	private:
		void SetupModels();
		void SetupParticles();
		void SetupPhysics();
	};
}
