//External includes
#include "SDL.h"
#undef main

//Standard includes
#include <iostream>

//Project includes
#include "Timer.h"
#include "Renderer.h"
#include "Scene.h"

using namespace dae;

void ShutDown(SDL_Window* pWindow)
{
	SDL_DestroyWindow(pWindow);
	SDL_Quit();
}

int main(int argc, char* args[])
{
	//Unreferenced parameters
	(void)argc;
	(void)args;
	
	//Create window + surfaces
	SDL_Init(SDL_INIT_VIDEO);
	constexpr uint32_t width = 640;
	constexpr uint32_t height = 480;

	//const auto pScene = new Scene_W1();
	//const auto pScene = new Scene_W2();
	//const auto pScene = new Scene_W3();
	const auto pScene = new Scene_W4();

	//const auto pScene = new Scene_W4_Bunny();


	pScene->Initialize();
	std::string title = "RayTracer - Xander Berten (2DAE09) - ";
	title += pScene->GetSceneName();
	
	SDL_Window* pWindow = SDL_CreateWindow(
		title.c_str(),
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		width, height, 0);

	if (!pWindow)
		return 1;

	//Initialize "framework"
	const auto pTimer = new Timer();
	const auto pRenderer = new Renderer(pWindow);


	

	//Start loop
	pTimer->Start();

	// Start Benchmark
	// pTimer->StartBenchmark();

	float printTimer = 0.f;
	bool isLooping = true;
	bool takeScreenshot = false;

	while (isLooping)
	{
		//--------- Get input events ---------
		SDL_Event e;
		while (SDL_PollEvent(&e))
		{
			switch (e.type)
			{
			case SDL_QUIT:
				isLooping = false;
				break;
			case SDL_KEYUP:

				if (e.key.keysym.scancode == SDL_SCANCODE_X) takeScreenshot = true;
				if (e.key.keysym.scancode == SDL_SCANCODE_F2) pRenderer->ToggleShadows();
				if (e.key.keysym.scancode == SDL_SCANCODE_F3) pRenderer->CycleLightingMode();
				if (e.key.keysym.scancode == SDL_SCANCODE_F6) pTimer->StartBenchmark();

				break;
			}
		}

		//--------- Update ---------
		pScene->Update(pTimer);

		//--------- Render ---------
		pRenderer->Render(pScene);

		//--------- Timer ---------
		pTimer->Update();
		printTimer += pTimer->GetElapsed();
		if (printTimer >= 1.f)
		{
			printTimer = 0.f;
			std::cout << "dFPS: " << pTimer->GetdFPS() << std::endl;
		}

		//Save screenshot after full render
		if (takeScreenshot)
		{
			if (!pRenderer->SaveBufferToImage())
				std::cout << "Screenshot saved!" << std::endl;
			else
				std::cout << "Something went wrong. Screenshot not saved!" << std::endl;
			takeScreenshot = false;
		}
	}
	pTimer->Stop();

	//Shutdown "framework"
	delete pScene;
	delete pRenderer;
	delete pTimer;

	ShutDown(pWindow);
	return 0;
}