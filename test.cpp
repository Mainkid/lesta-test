
#include <thread>
#include <mutex>
#include <atomic>
#include <iostream>
#include <vector>
#include <stack>
#include <queue>
#include <set>
#include <unordered_set>
#include <unordered_map>

#include "./nvToolsExt.h"

#include "test.h"
#include "Particle.h"
#include "ParticleSystem.h"



std::unique_ptr<ParticleSystem> particleSystem;
std::unique_ptr<Buffer> renderBuffer;

static std::atomic_int globalTime;
static std::atomic_bool workerMustExit = false;



void WorkerThread(void)
{

	while (!workerMustExit)
	{
		nvtxRangePush(__FUNCTION__);

		static int lastTime = 0;
		const int time = globalTime.load();
		int delta = time - lastTime;
		lastTime = time;

		if (delta > 0)
		{
			// some code
			clickMutex.lock();
			particleSystem->particlesEmit();
			clickMutex.unlock();

			particleSystem->particlesUpdate(delta * 0.001f);

			renderMutex.lock();
			particleSystem->copyToRenderBuffer(renderBuffer.get());
			renderMutex.unlock();
		}

		static const int MIN_UPDATE_PERIOD_MS = 10;
		if (delta < MIN_UPDATE_PERIOD_MS)
			std::this_thread::sleep_for(std::chrono::milliseconds(MIN_UPDATE_PERIOD_MS - delta));

		nvtxRangePop();
	}
}


void test::init(void)
{
	srand(time(0));
	
	particleSystem = std::make_unique<ParticleSystem>(Vector2{test::SCREEN_WIDTH, test::SCREEN_HEIGHT});
	renderBuffer = std::make_unique<Buffer>();

	std::thread workerThread(WorkerThread);
	workerThread.detach();
}

void test::term(void)
{
	renderBuffer = nullptr;
	particleSystem = nullptr;
	workerMustExit = true;
}

void test::render(void)
{
	if (!renderBuffer)
		return;

	std::lock_guard<std::mutex> renderGuard(renderMutex);

	for (size_t index = 0; index < renderBuffer->size; ++index)
	{
		Vector2& position = renderBuffer->position[index];
		Color& color = renderBuffer->color[index];

		platform::drawPoint(position.x, position.y,
			color.r(), color.g(), color.b(), color.a());
	}
}

void test::update(int dt)
{
	globalTime.fetch_add(dt);
}

void test::on_click(int x, int y)
{

	std::lock_guard<std::mutex> clickGuard(clickMutex);

	if (particleSystem)
		particleSystem->addToSpawnList(Vector2{ x * 1.0f, test::SCREEN_HEIGHT - y * 1.0f });
}