// Author: Conor Meehan
// Creates particles that will be updated on the graphics card allowing a much greater numeber
// to be updated in a time step.
#pragma once
#include "GameComponent.h"
#include <vector>
#include <string>

namespace BGE
{
	class GPUParticleEffect
		:	public GameComponent
	{
	public:
		GPUParticleEffect(void);
		~GPUParticleEffect(void);

		bool Initialise();
		void Update(float timeDelta);
		void Draw();

		void ComputeEmitter( float timeDelta );
		void ComputeIntegration( float timeDelta );
		const char * emitShaderName;
		const char * integrationShaderName;

		float life;
		boolean isDead;

		std::vector<glm::vec3> vertices;
		std::vector<glm::vec3> velocities;
		std::vector<glm::vec3> lives;
		std::vector<glm::vec4> colours;
		std::vector<float> sizes;
		float size;
		glm::vec4 startColor;
		glm::vec4 endColor;

		std::string textureName;

		int maxParticles;
		float maxLife;
		GLuint programID;
		GLuint integrateProgramID;
		GLuint emitProgramID;
		GLuint colourBufferID;
		GLuint vao;
		GLuint positionBufferID;
		GLuint velocityBufferID;
		GLuint lifeBufferID;
		GLuint sizeBufferID;
		GLuint mID, vID, pID;
		GLuint pointSizeID;
		GLuint textureID;
		GLuint textureSampler;

		GLuint dtLocation;
		GLuint gravityLocation;
		glm::vec3 gravity;
		GLuint attractorLocation;
		glm::vec3 attractor;
		float attractorScale;
		GLuint attractorFlagLocation;
		glm::vec3 emitter;
		glm::vec3 emitterSize;
		GLuint emitterLocation;
		GLuint emitterSizeLocation;
		GLuint seedLocation;
		GLuint startColorLocation;
		GLuint endColorLocation;
		GLuint maxLifeLocation;
		GLuint integrationNumberParticlesLocation;
		GLuint emitterNumberParticlesLocation;
	};
}