#pragma once

#include <cstdint>

#include "ColorRGB.h"
#include "Material.h"
#include "Vector3.h"


struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	class Scene;

	class Renderer final
	{
	public:
		Renderer(SDL_Window* pWindow);
		~Renderer() = default;

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		void Render(Scene* pScene) const;
		bool SaveBufferToImage() const;

	private:
		Vector3 GetRayDirection(float x, float y) const;
		void RenderChunk(int startPx, int endPx, const Scene* pScene, const std::vector<Material*>& materials) const;

		SDL_Window* m_pWindow{};
		SDL_Surface* m_pBuffer{};
		uint32_t* m_pBufferPixels{};

		float m_AspectRatio{};

		int m_Width{};
		int m_Height{};

		constexpr int m_ThreadCount{ 8 };

	};
}
