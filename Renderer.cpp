//External includes
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include "Renderer.h"
#include "Scene.h"
#include "Utils.h"
#include "ChunkRendererManager.h"

using namespace dae;

Renderer::Renderer(SDL_Window* pWindow, Scene* pScene) :
	m_pWindow(pWindow),
	m_pBuffer(SDL_GetWindowSurface(pWindow)),
	m_pScene(pScene),
	m_ChunkRendererManager(&ChunkRendererManager::GetInstance())
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);
	m_pBufferPixels = static_cast<uint32_t*>(m_pBuffer->pixels);
	m_AspectRatio = static_cast<float>(m_Width) / static_cast<float>(m_Height);

	m_ChunkRendererManager->CalculateChunks(pScene, m_Width, m_Height, m_AspectRatio);
	m_ChunkRendererManager->CreateThreads();
	m_ChunkRendererManager->SetBuffer(m_pBuffer, m_pBufferPixels);
}




void Renderer::Render(Scene* /*pScene*/) const
{
	while(!m_ChunkRendererManager->IsDone())
	{
		//this threads sleeps for 1 ms
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	m_ChunkRendererManager->RenderNewFrame();

	//@END
	//Update SDL Surface
	SDL_UpdateWindowSurface(m_pWindow);
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBuffer, "RayTracing_Buffer.bmp");
}

void Renderer::CycleLightingMode() const
{
	//Increment the lighting mode
	m_ChunkRendererManager->CycleLightingMode();
}

void Renderer::ToggleShadows() const
{
	m_ChunkRendererManager->ToggleShadows();
}

