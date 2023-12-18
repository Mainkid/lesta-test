#pragma once
#include <vector>
#include <thread>
#include <mutex>
#include <memory>
#include <atomic>

#include "Particle.h"


/*
 *	Particle System settings
 */

#define MAX_EFFECTS_AMOUNT					2048u
#define NUM_PARTICLES_TO_EMIT				64u
#define MAX_PARTICLES_AMOUNT				MAX_EFFECTS_AMOUNT * NUM_PARTICLES_TO_EMIT
#define SECOND_PARTICLE_SPAWN_PROBABILITY	0.25f

#define MAX_PARTICLE_SPEED					100.0f
#define MIN_PARTICLE_SPEED					0.05f
#define MAX_PARTICLE_LIFETIME				3.0f
#define MIN_PARTICLE_LIFETIME				2.0f
#define PARTICLE_COLOR						0x00ff00

static const Vector2 gravity = { 0,-100 };
static std::mutex clickMutex;
static std::mutex renderMutex;


struct Buffer {


public:
	Color* color;
	Vector2* position;
	size_t size;

	Buffer()
	{
		color = new Color[MAX_PARTICLES_AMOUNT];
		position = new Vector2[MAX_PARTICLES_AMOUNT];
		size = 0;
	}

	~Buffer()
	{
		delete[] color;
		delete[] position;
	}

};



struct ParticleSystem : public Buffer
{
private:
	Particle** particlesPool;									// <--- Effective way of swapping particles via pointers
	Vector2 screenSize;
	std::vector<Vector2> spawnList;

public:
	ParticleSystem(Vector2 _screenSize) : Buffer()
	{
		particlesPool = new Particle * [MAX_PARTICLES_AMOUNT];
		screenSize = _screenSize;

		for (size_t i = 0; i < MAX_PARTICLES_AMOUNT; i++)
		{
			particlesPool[i] = new Particle();
		}

	}

	~ParticleSystem()
	{
		for (int i = 0; i < MAX_PARTICLES_AMOUNT; i++)
			delete particlesPool[i];

		delete[] particlesPool;
	}

	void initParticle(Vector2 startLocation, size_t index)
	{
		position[index] = startLocation;
		color[index].value = PARTICLE_COLOR;

		particlesPool[index]->isAlive = true;
		particlesPool[index]->velocityVec = Vector2{ rand() % 1000 - 500.0f, rand() % 1000 - 500.0f };
		particlesPool[index]->velocityVec.normalize();
		particlesPool[index]->lifeTime = MIN_PARTICLE_LIFETIME +
			(MAX_PARTICLE_LIFETIME - MIN_PARTICLE_LIFETIME) * (rand() % 100 / 100.0f);
		particlesPool[index]->startLifeTime = particlesPool[index]->lifeTime;
		particlesPool[index]->speed = MIN_PARTICLE_SPEED +
			(MAX_PARTICLE_SPEED - MIN_PARTICLE_SPEED) * (rand() % 100 / 100.0f);
	}

	void particlesEmit()
	{
		if (spawnList.empty())
			return;

		size_t indexHelper = 0;

		for (size_t i = size; i < MAX_PARTICLES_AMOUNT; ++i)
		{

			initParticle(spawnList[indexHelper / NUM_PARTICLES_TO_EMIT], i);
			++size;
			++indexHelper;

			if (indexHelper >= spawnList.size() * NUM_PARTICLES_TO_EMIT)
				break;
		}

		spawnList.clear();
	}

	void freeParticle(size_t index)
	{
		particlesPool[index]->isAlive = false;

		std::swap(particlesPool[index], particlesPool[size - 1]);
		std::swap(color[index], color[size - 1]);
		std::swap(position[index], position[size - 1]);

		--size;
	}

	void particlesUpdate(float deltaTime)
	{
		for (size_t i = 0; i < size; ++i)
		{

			bool& isAlive = particlesPool[i]->isAlive;

			if (!isAlive)
			{
				freeParticle(i);
				--i;
				continue;
			}

			Particle& particle = *particlesPool[i];
			Vector2& position_ = position[i];
			Color& color_ = color[i];


			if (particle.lifeTime <= 0.0f)
			{

				if (rand() % 100 / 100.0f < SECOND_PARTICLE_SPAWN_PROBABILITY)
				{
					clickMutex.lock();
					spawnList.push_back(position_);
					clickMutex.unlock();
				}
				freeParticle(i);
				--i;
				continue;
			}
			else if (position_.x > screenSize.x || position_.x < 0 ||
				position_.y > screenSize.y || position_.y < 0)
			{
				freeParticle(i);
				--i;
				continue;
			}

			color_.setAlpha(particle.lifeTime / particle.startLifeTime);

			position_ +=
				(particle.velocityVec * particle.speed +
					gravity * (particle.startLifeTime - particle.lifeTime)
					) * deltaTime;

			particle.lifeTime -= deltaTime;
		}
	}

	void addToSpawnList(Vector2 location)
	{
		spawnList.push_back(location);
	}

	void copyToRenderBuffer(Buffer* renderBuffer)
	{
		renderBuffer->size = this->size;
		std::memcpy(renderBuffer->color, this->color, sizeof(Color) * renderBuffer->size);
		std::memcpy(renderBuffer->position, this->position, sizeof(Vector2) * renderBuffer->size);
	}
};

