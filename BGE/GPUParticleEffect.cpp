#include "GPUParticleEffect.h"
#include "Content.h"
#include "Utils.h"

using namespace BGE;

void CheckGlError( std::string errorMessage )
{
	if (GLenum err = glGetError() != GL_NO_ERROR) {
		std::cout << "ERROR: " << err  << " : " << errorMessage << std::endl;
		//TODO gluErrorString(err) breaks
		exit (-1);
		std::cin.get();
	}
}

GPUParticleEffect::GPUParticleEffect(void)
{
	emitShaderName = "particleEmitter";
	integrationShaderName = "particle";

	maxParticles = 10000;
	maxLife = 1.0;
	textureName = "baseParticle2";
	tag = "GPUParticleEffect";

	gravity = glm::vec3(0,-1.0f,0);
	attractor = glm::vec3(0, 0, 0);
	attractorScale = 0.0f;
	emitter = glm::vec3(0,10,-10);
	emitterSize = glm::vec3(0.2);

	startColor = glm::vec4(1.0, 0.1, 0.1, 0.5);
	endColor = glm::vec4(1.0, 0.8, 0.9, 0.5);

	size = 20.0;
}

GPUParticleEffect::~GPUParticleEffect(void)
{
}

bool GPUParticleEffect::Initialise()
{
	if (initialised)
	{
		return true;
	}

	if (! GL_VERSION_4_3)
	{
		throw BGE::Exception("OpenGL compute Shaders not supported. Particles will not work. Upgrade your graphics card drivers");
	}

	// Setup the compute shader programs
	emitProgramID =  Content::LoadComputeShader(emitShaderName);
	emitterLocation = glGetUniformLocation(emitProgramID, "emitter");
	emitterSizeLocation = glGetUniformLocation(emitProgramID, "emitterSize");
	maxLifeLocation = glGetUniformLocation(emitProgramID, "maxLife");
	seedLocation = glGetUniformLocation(emitProgramID, "seed");
	emitterNumberParticlesLocation = glGetUniformLocation(emitProgramID, "numberParticles");

	integrateProgramID =  Content::LoadComputeShader(integrationShaderName);
	dtLocation = glGetUniformLocation(integrateProgramID, "dt");
	gravityLocation = glGetUniformLocation(integrateProgramID, "g");
	startColorLocation = glGetUniformLocation(integrateProgramID, "startColor");
	endColorLocation = glGetUniformLocation(integrateProgramID, "endColor");
	attractorFlagLocation  = glGetUniformLocation(integrateProgramID, "attractorScale");
	integrationNumberParticlesLocation = glGetUniformLocation(integrateProgramID, "numberParticles");

	// Init lives to 0 and sizes to size
	lives = vector<glm::vec3>(maxParticles, 0);
	sizes = vector<float>(maxParticles, size);

	// Load the program for drawing
	programID = Content::LoadShaderPair("Particles");

	glUseProgram(programID);
	glBindVertexArray(0);

	glGenBuffers(1, &positionBufferID);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, positionBufferID);
	glBufferData(GL_SHADER_STORAGE_BUFFER, maxParticles * sizeof(glm::vec3), NULL/*&vertices[0]*/, GL_DYNAMIC_DRAW);	

	glGenBuffers(1, &velocityBufferID);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, velocityBufferID);
	glBufferData(GL_SHADER_STORAGE_BUFFER, maxParticles * sizeof(glm::vec3), NULL/*&velocities[0]*/, GL_DYNAMIC_DRAW);	
	
	glGenBuffers(1, &lifeBufferID);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, lifeBufferID);
	glBufferData(GL_SHADER_STORAGE_BUFFER, maxParticles * sizeof(glm::vec3), &lives[0], GL_DYNAMIC_DRAW);

	glGenBuffers(1, &attractorLocation);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, attractorLocation);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec3), &attractor[0], GL_DYNAMIC_DRAW);

	mID = glGetUniformLocation(programID,"m");
	vID = glGetUniformLocation(programID,"v");
	pID = glGetUniformLocation(programID,"p");

	glGenBuffers(1, &colourBufferID);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, colourBufferID);
	glBufferData(GL_SHADER_STORAGE_BUFFER, maxParticles * sizeof(glm::vec4), NULL/*&colours[0]*/, GL_DYNAMIC_DRAW);

	glGenBuffers(1, &sizeBufferID);
	glBindBuffer(GL_ARRAY_BUFFER, sizeBufferID);
	glBufferData(GL_ARRAY_BUFFER, maxParticles * sizeof(float), &sizes[0], GL_DYNAMIC_DRAW);

	textureSampler  = glGetUniformLocation(programID, "myTextureSampler");
	textureID = Content::LoadTexture(textureName);

	glUseProgram(0);
	initialised = true;

	life = 0;
	isDead = false;

	return GameComponent::Initialise();
}

void GPUParticleEffect::Update( float timeDelta )
{
	// Here we integrate the particles in a compute shader
	GameComponent::Update(timeDelta);
	glGetError();

	ComputeEmitter(timeDelta);
	ComputeIntegration(timeDelta);

	life += timeDelta;
	if(life < maxLife)
	{
		life += timeDelta;
	}
	else
	{
		isDead = true;
	}
}

void GPUParticleEffect::Draw()
{
	glUseProgram(programID);
	// Disable depth mask to prevent a problem of sharp edges on particles due to a zbuffer problem
	// However this leads to the ocasional probelm of out of order particles
	glDepthMask(GL_FALSE);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glUniform1i(textureSampler, 0);

	glEnable(GL_BLEND);	
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_POINT_SPRITE);
	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Copy the data into the buffers for drawing
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, positionBufferID);

	glVertexAttribPointer(
		0,                  // attribute
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
		);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, colourBufferID);
	glVertexAttribPointer(
		1,                  // attribute
		4,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
		);


	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, sizeBufferID);
	glVertexAttribPointer(
		2,                  // attribute
		1,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
		);

	// The particles are already in world space, so we wont be using this anyway

	glUniformMatrix4fv(mID, 1, GL_FALSE, & world[0][0]);
	glUniformMatrix4fv(vID, 1, GL_FALSE, & Game::Instance()->camera->view[0][0]);
	glUniformMatrix4fv(pID, 1, GL_FALSE, & Game::Instance()->camera->projection[0][0]);

	glUniform1f(pointSizeID, 20.0f);

	glDrawArrays(GL_POINTS, 0, maxParticles);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glUseProgram(0);
	glDepthMask(GL_TRUE);

	return GameComponent::Draw();
}

/// Advance the particles using a compute shader.
void GPUParticleEffect::ComputeIntegration( float timeDelta )
{
	glUseProgram(integrateProgramID);
	glUniform1ui(integrationNumberParticlesLocation, maxParticles);
	glUniform1f(dtLocation, timeDelta);
	glUniform3f(gravityLocation, gravity.x, gravity.y, gravity.z);
	glUniform1f(attractorFlagLocation, attractorScale);
	glUniform4f(startColorLocation, startColor.x, startColor.y, startColor.z, startColor.w);
	glUniform4f(endColorLocation, endColor.x, endColor.y, endColor.z, endColor.w);


	CheckGlError("Use integrator program error\n");
	glEnableVertexAttribArray(0);
	glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 0, positionBufferID );
	glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 1, velocityBufferID );
	glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 2, lifeBufferID );
	glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 3, colourBufferID );
	glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 4, attractorLocation );
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec3), &attractor[0], GL_DYNAMIC_DRAW);

	CheckGlError("Program, buffer bind.");

	glDispatchCompute(maxParticles/256+1, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
	CheckGlError("Integrator Dispatch.");
	glUseProgram(0);
}

/// Emits new particles when the old ones die.
void BGE::GPUParticleEffect::ComputeEmitter( float timeDelta )
{
	glUseProgram(emitProgramID);
	glUniform1ui(emitterNumberParticlesLocation, maxParticles);
	glUniform3f(emitterLocation, emitter.x, emitter.y, emitter.z);
	glUniform3f(emitterSizeLocation, emitterSize.x, emitterSize.y, emitterSize.z);
	glUniform1f(seedLocation, RandomFloat());
	glUniform1f(maxLifeLocation, maxLife);

	CheckGlError("Use emitter program error\n");
	glEnableVertexAttribArray(0);
	glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 0, positionBufferID );
	glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 1, velocityBufferID );
	glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 2, lifeBufferID );

	glDispatchCompute(maxParticles/256+1, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
	CheckGlError("Emitter Dispatch.");
	glUseProgram(0);
}
