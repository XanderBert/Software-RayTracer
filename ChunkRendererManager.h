#pragma once
#include <thread>
#include <vector>
#include "ChunkRenderer.h"
#include "Renderer.h"
#include "Singleton.h"

enum class LightingMode
{
	ObservedArea,
	Radiance,
	BRDF,
	Combined,
	//@end
	COUNT

};



namespace dae
{
	class Scene;
	class Renderer;


	class ChunkRendererManager : public Singleton<ChunkRendererManager>
	{
	public:
		~ChunkRendererManager() override;

		ChunkRendererManager(const ChunkRendererManager&) = delete;
		ChunkRendererManager(ChunkRendererManager&&) noexcept = delete;
		ChunkRendererManager& operator=(const ChunkRendererManager&) = delete;
		ChunkRendererManager& operator=(ChunkRendererManager&&) noexcept = delete;

		void RenderNewFrame();


		void CalculateChunks(Scene* pScene, int width, int height, float aspectRatio);
		void CreateThreads();
		void SetBuffer(SDL_Surface* pBuffer, uint32_t* pBufferPixels);

		bool IsDone() const;


		void CycleLightingMode();
		LightingMode GetLightingMode() const { return m_LightingMode; }

		void ToggleShadows();
		bool ShadowsEnabled() const { return m_ShadowsEnabled; }

		int GetWidth() const;
		int GetHeight() const;
		float GetAspectRatio() const;

		SDL_Surface* GetBuffer() const;
		uint32_t* GetBufferPixels() const;

	private:
		friend class Singleton<ChunkRendererManager>;
		ChunkRendererManager() = default;

		const unsigned int m_ProcessorCount = std::thread::hardware_concurrency();
		float m_ChunkSize{ 0 };

		std::vector<std::thread> m_Threads;
		std::vector<ChunkRenderer> m_Renderers;

		Scene* m_pScene{};

		bool m_ShadowsEnabled{ true };
		LightingMode m_LightingMode{ LightingMode::Combined };

		int m_Width;
		int m_Height;
		float m_AspectRatio;


		SDL_Surface* m_pBuffer{};
		uint32_t* m_pBufferPixels{};
	};
}