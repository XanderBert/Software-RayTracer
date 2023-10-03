#include "ChunkRendererManager.h"
#include <assert.h>
#include <iostream>
#include "ChunkRenderer.h"
#include "Scene.h"
#include "Renderer.h"

using namespace dae;
ChunkRendererManager::~ChunkRendererManager()
{
	for (std::thread& thread : m_Threads)
	{
		thread.join();
	}
}

void ChunkRendererManager::RenderNewFrame()
{
	for (dae::ChunkRenderer& renderer : m_Renderers)
	{
		renderer.Render();
	}
}

void ChunkRendererManager::CalculateChunks(dae::Scene* pScene, int width, int height, float aspectRatio)
{
	m_ChunkSize = static_cast<float>(width) / static_cast<float>(m_ProcessorCount);


	m_pScene = pScene;
	m_Width = width;
	m_Height = height;
	m_AspectRatio = aspectRatio;


	//assert if chunkSize is not a whole number
	assert(m_ChunkSize == static_cast<float>(static_cast<int>(m_ChunkSize)));

}

void ChunkRendererManager::CreateThreads()
{
	assert(m_ChunkSize != 0 && "Calculate the Chunks first");

	// Create and add ChunkRenderer objects to the container
	for (unsigned int i = 0; i < m_ProcessorCount; ++i)
	{
		m_Renderers.emplace_back();

		//[0,40]
		//[40, 80]
		//Todo Fix this so that some lines do not get renderer twice
		//Todo there must be some race condition or sth because there is some kind of screen tearing
		const auto start = static_cast<int>(static_cast<unsigned int>(m_ChunkSize) * i);
		const auto end = static_cast<int>(start + m_ChunkSize);

		m_Renderers[i].SetChunk(start, end, m_pScene);
	}


	for (unsigned int i = 0; i < m_Renderers.size(); ++i)
	{
		m_Threads.emplace_back(std::thread(std::ref(m_Renderers[i])));
	}
}

void ChunkRendererManager::SetBuffer(SDL_Surface* pBuffer, uint32_t* pBufferPixels)
{
	m_pBuffer = pBuffer;
	m_pBufferPixels = pBufferPixels;
}

bool ChunkRendererManager::IsDone() const
{
	for (const dae::ChunkRenderer& renderer : m_Renderers)
	{
		if (!renderer.IsDone())
		{
			return false;
		}
	}

	return true;
}

void ChunkRendererManager::CycleLightingMode()
{
	m_LightingMode = static_cast<LightingMode>((static_cast<int>(m_LightingMode) + 1) % static_cast<int>(LightingMode::COUNT));
}

void ChunkRendererManager::ToggleShadows()
{
	m_ShadowsEnabled = !m_ShadowsEnabled;
}

int ChunkRendererManager::GetWidth() const
{
	return m_Width;
}

int ChunkRendererManager::GetHeight() const
{
	return m_Height;
}

float ChunkRendererManager::GetAspectRatio() const
{
	return m_AspectRatio;
}

SDL_Surface* ChunkRendererManager::GetBuffer() const
{
	return m_pBuffer;
}

uint32_t* ChunkRendererManager::GetBufferPixels() const
{
	return m_pBufferPixels;
}
