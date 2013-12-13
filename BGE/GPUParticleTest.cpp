#include "GPUParticleTest.h"
#include "Content.h"
#include "VectorDrawer.h"
#include "PhysicsCamera.h"

using namespace BGE;

GPUParticleTest::GPUParticleTest(void)
{
	elapsed = 10000;
	timecount = 1;

	handTimeForLerp = 0.2;
	handTime = 0;
	handStartOrientation = glm::angleAxis(90.0f, glm::vec3(1,0,0));
	handEndOrientation = glm::angleAxis(90.0f, glm::vec3(0.5,0.5,-0.4));
}


GPUParticleTest::~GPUParticleTest(void)
{
}

bool GPUParticleTest::Initialise()
{
	SetupModels();
	SetupParticles();
	SetupPhysics();

	shared_ptr<PhysicsController> box1 = physicsFactory->CreateBox(1,1,4, glm::vec3(5, 5, 0), glm::quat()); 

	riftEnabled = false;
	fullscreen = false;
	width = 800;
	height = 600;

	Game::Initialise();

	camera->GetController()->position = glm::vec3(0, 4, 20);
	return true;
}

void GPUParticleTest::Update(float timeDelta)
{
	Game::Update(timeDelta);

	dynamicsWorld->stepSimulation(timeDelta,100);
	
	thrusterL->emitter = ship1->position+ship1->look*glm::vec3(-0.55)+ship1->right*glm::vec3(-0.34)+ship1->up*glm::vec3(-0.1);
	thrusterL->attractor = ship1->position+ship1->look*glm::vec3(-5);

	thrusterR->emitter = ship1->position+ship1->look*glm::vec3(-0.55)+ship1->right*glm::vec3(0.19)+ship1->up*glm::vec3(-0.1);
	thrusterR->attractor = ship1->position+ship1->look*glm::vec3(-5);

	if (keyState[SDL_SCANCODE_N])
	{
		stayAliveParticles->attractorScale -= 0.5;
	}	
	if (keyState[SDL_SCANCODE_M])
	{
		stayAliveParticles->attractorScale += 0.5;
	}
	if (keyState[SDL_SCANCODE_B])
	{
		stayAliveParticles->attractorScale = 0.0;
	}

	// Movement of ship1
	if (keyState[SDL_SCANCODE_U])
	{
		ship1->position += ship1->look * speed * timeDelta;
	}
	if (keyState[SDL_SCANCODE_J])
	{
		ship1->position -= ship1->look * speed * timeDelta;
	}
	if (keyState[SDL_SCANCODE_H])
	{
		ship1->Yaw(timeDelta * speed * speed);
	}
	if (keyState[SDL_SCANCODE_K])
	{
		ship1->Yaw(-timeDelta * speed * speed);
	}

	
	// This section lerps the hand between the start and end position.
	if(handTime < 0)
	{
		handTime = 0;
	}
	if(handTime > handTimeForLerp)
	{
		handTime = handTimeForLerp;
	}

	hand->orientation = glm::mix(handStartOrientation, handEndOrientation, handTime/handTimeForLerp);

	// This section controls the particle effect that shoots from the player when clicked
	if(toggleParticles != NULL)
	{
		toggleParticles->emitter = Game::camera->position + Game::camera->look*0.7f + Game::camera->right*-0.2f + Game::camera->up*-0.2f;
		toggleParticles->attractor = Game::camera->position + Game::camera->look*10.0f;
		// Check if there is an object attached to the camera and if so set the attractor to that position
		std::list<std::shared_ptr<GameComponent>>* l = Game::camera->GetChildren();
		std::list<std::shared_ptr<GameComponent>>::const_iterator iterator;
		for (iterator = l->begin(); iterator != l->end(); ++iterator)
		{
			std::shared_ptr<GameComponent> c = *iterator;
			if(c->tag == "Physics Camera")
			{
				std::shared_ptr<PhysicsCamera> physicsCamera = dynamic_pointer_cast<PhysicsCamera, GameComponent>(c);
				if(physicsCamera->pickedUp != NULL)
				{
					toggleParticles->attractor = physicsCamera->pickedUp->position - Game::camera->look*4.0f;
				}
			}
		}
	}

	if(SDL_BUTTON(SDL_GetMouseState(NULL,NULL)) == SDL_BUTTON_LEFT)
	{
		handTime += timeDelta;
		if(toggleParticles == NULL)
		{
			toggleParticles = make_shared<GPUParticleEffect>();
			toggleParticles->size = 1.2;
			toggleParticles->startColor = glm::vec4(0.1, 0.5, 0.9, 0.6);
			toggleParticles->endColor = glm::vec4(0.5, 0.6, 0.1, 0.7);
			toggleParticles->emitter = Game::camera->position + Game::camera->look*0.7f + Game::camera->right*-0.2f + Game::camera->up*-0.2f;
			toggleParticles->attractor = Game::camera->position + Game::camera->look*10.0f;
			toggleParticles->attractorScale = 70.0f;
			toggleParticles->emitterSize = glm::vec3(0.1,0.1,0.1);
			toggleParticles->Initialise();
			Attach(toggleParticles);
		}
	}
	else
	{
		handTime -= timeDelta;
		if(toggleParticles != NULL)
		{
			if(toggleParticles->isDead == true)
			{
				toggleParticles->alive = false;
				toggleParticles = NULL;
			}
		}
	}

	// Update the snow
	if(SDL_BUTTON(SDL_GetMouseState(NULL,NULL)) == 2)
	{
		snow->attractor = Game::camera->position;
		snow->attractorScale = 80;
	}
	else
	{
		snow->attractorScale = 0;
	}
	snow->emitter = Game::camera->position + GameComponent::basisUp * 5.0f + GameComponent::basisLook * 50.0f - GameComponent::basisRight * 50.0f;
}

void GPUParticleTest::SetupModels()
{
	std::shared_ptr<GameComponent> ground = make_shared<Ground>();
	Attach(ground);

	ship1 = make_shared<GameComponent>();
	ship1->Attach(Content::LoadModel("cobramk3", glm::rotate(glm::mat4(1), 180.0f, glm::vec3(0,1,0))));
	//ship1->Attach(make_shared<VectorDrawer>(glm::vec3(5,5,5)));
	ship1->position = glm::vec3(-10, 2, -10);
	Attach(ship1);
	
	candle = make_shared<GameComponent>();
	candle->Attach(Content::LoadModel("candle", glm::rotate(glm::mat4(1), 180.0f, glm::vec3(0,1,0))));
	//candle->Attach(make_shared<VectorDrawer>(glm::vec3(5,5,5)));
	candle->position = glm::vec3(0, 0, -10.1);
	candle->scale= glm::vec3(2.3);
	Attach(candle);
	
	hand = make_shared<GameComponent>();
	hand->Attach(Content::LoadModel("hand2", glm::rotate(glm::mat4(1), 180.0f, glm::vec3(-0.7,1,-0.4))));
	hand->position = glm::vec3(-0.1, -0.1, -0.3);
	hand->orientation = glm::angleAxis(90.0f, glm::vec3(1,0,0));
	hand->scale = glm::vec3(0.3);
	hand->worldMode = world_modes::from_self_with_parent;
	Game::camera->Attach(hand);
}

void GPUParticleTest::SetupParticles()
{
	thrusterL = make_shared<GPUParticleEffect>();
	thrusterL->maxLife = 0.3;
	thrusterL->startColor = glm::vec4(0.9, 0.1, 0.1, 0.3);
	thrusterL->endColor = glm::vec4(0.7, 0.5, 0.5, 0.1);
	thrusterL->gravity = glm::vec3(0,0.2,0);
	thrusterL->emitter = ship1->position+ship1->look*glm::vec3(-0.55)+ship1->right*glm::vec3(-0.34)+ship1->up*glm::vec3(-0.1);
	thrusterL->attractor = ship1->position+ship1->look*glm::vec3(-1);
	thrusterL->attractorScale = 5.0;
	Attach(thrusterL);

	thrusterR = make_shared<GPUParticleEffect>();
	thrusterR->maxLife = 0.3;
	thrusterR->startColor = glm::vec4(0.9, 0.1, 0.1, 0.3);
	thrusterR->endColor = glm::vec4(0.7, 0.5, 0.5, 0.1);
	thrusterR->gravity = glm::vec3(0,0.2,0);
	thrusterR->emitter = ship1->position+ship1->look*glm::vec3(-0.55)+ship1->right*glm::vec3(0.34)+ship1->up*glm::vec3(-0.1);
	thrusterR->attractor = ship1->position+ship1->look*glm::vec3(-1);
	thrusterR->attractorScale = 5.0;
	Attach(thrusterR);

	particles2 = make_shared<GPUParticleEffect>();
	particles2->maxParticles = 100000;
	particles2->size = 4;
	particles2->emitter = glm::vec3(-0.13, 4.25, -10.2);
	particles2->attractor = glm::vec3(0, 7, -10);
	particles2->attractorScale = 3.0;
	particles2->emitterSize = glm::vec3(0.2,0.5,0.2);
	Attach(particles2);

	snow = make_shared<GPUParticleEffect>();
	snow->textureName = "flare";
	snow->maxParticles = 1000000;
	snow->size = 10;
	// Draw the first particles offscreen
	snow->emitter = glm::vec3(1000000);
	snow->attractorScale = 0.0;
	snow->startColor = glm::vec4(1,1,1,0.6);
	snow->endColor = glm::vec4(0.8,0.8,0.8,0.5);
	snow->emitterSize = glm::vec3(100,15,100);
	snow->maxLife = 10;
	Attach(snow);

	stayAliveParticles = make_shared<GPUParticleEffect>();
	stayAliveParticles->emitterSize = glm::vec3(1,0.2,1);
	stayAliveParticles->maxLife = -1.0;
	stayAliveParticles->startColor = glm::vec4(0.0, 0.5, 0.0, 0.5);
	stayAliveParticles->endColor = glm::vec4(0.1, 0.6, 0.3, 0.3);
	stayAliveParticles->maxParticles = 1000000;
	stayAliveParticles->emitter = glm::vec3(-5, 15, -10);
	stayAliveParticles->attractor = glm::vec3(5, 15, -15);
	stayAliveParticles->attractorScale = 2.0;
	stayAliveParticles->size = 1;
	Attach(stayAliveParticles);
}

void GPUParticleTest::SetupPhysics()
{
	// Set up the collision configuration and dispatcher
    collisionConfiguration = new btDefaultCollisionConfiguration();
    dispatcher = new btCollisionDispatcher(collisionConfiguration);
 
    // The world.
	btVector3 worldMin(-1000,-1000,-1000);
	btVector3 worldMax(1000,1000,1000);
	broadphase = new btAxisSweep3(worldMin,worldMax);
	solver = new btSequentialImpulseConstraintSolver();
	dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher,broadphase,solver,collisionConfiguration);
    dynamicsWorld->setGravity(btVector3(0,0,0));


	physicsFactory = make_shared<PhysicsFactory>(dynamicsWorld);

	physicsFactory->CreateGroundPhysics();
	physicsFactory->CreateCameraPhysics();
}