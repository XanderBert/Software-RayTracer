#pragma once
#include <vector>

struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	class Scene;
	class Material;

	struct Ray;
	struct Vector2;
	struct Vector3;
	struct Matrix;
	struct Camera;
	struct ColorRGB;

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

		void CycleLightingMode();
		void ToggleShadows() { m_ShadowsEnabled = !m_ShadowsEnabled; }

	private:
		Vector3 GetRayDirection(float x, float y, Camera* pCamera) const;
		void RenderPixel(Scene* pScene, const Vector2& rayLocation) const;
		void SetupPixelIndices();
		
		SDL_Window* m_pWindow{};
		SDL_Surface* m_pBuffer{};
		uint32_t* m_pBufferPixels{};
		uint32_t m_amountOfPixels{};
		std::vector<Vector2> m_PixelIndices{};
		
		float m_AspectRatio{};
		static constexpr float m_RayOffset{ 0.001f };

		int m_Width{};
		int m_Height{};





		enum class LightingMode
		{
			ObservedArea,
			Radiance,
			BRDF,
			Combined,
			//@end
			COUNT

		};
		bool m_ShadowsEnabled{ true };
		LightingMode m_LightingMode{ LightingMode::Combined };

		constexpr int static m_ThreadCount{ 8 };
	};
}