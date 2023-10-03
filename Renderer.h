#pragma once

struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	class ChunkRendererManager;
	class Scene;
	class Material;
	struct Vector3;
	struct Camera;

	class Renderer final
	{
	public:
		Renderer(SDL_Window* pWindow, Scene* pScene);
		~Renderer() = default;

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		void Render(Scene* pScene) const;
		bool SaveBufferToImage() const;

		void CycleLightingMode() const;
		void ToggleShadows() const;

	private:
		ChunkRendererManager* m_ChunkRendererManager{};
		SDL_Window* m_pWindow{};

		SDL_Surface* m_pBuffer{};
		uint32_t* m_pBufferPixels{};

		float m_AspectRatio{};
		int m_Width{};
		int m_Height{};

		Scene* m_pScene{};

	};
}