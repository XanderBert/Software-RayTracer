//External includes
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include "Renderer.h"
#include "Math.h"
#include "Matrix.h"
#include "Material.h"
#include "Scene.h"
#include "Utils.h"

using namespace dae;

Renderer::Renderer(SDL_Window * pWindow) :
	m_pWindow(pWindow),
	m_pBuffer(SDL_GetWindowSurface(pWindow))
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);
	m_pBufferPixels = static_cast<uint32_t*>(m_pBuffer->pixels);
}

void Renderer::Render(Scene* pScene) const
{
	Camera& camera = pScene->GetCamera();
	auto& materials = pScene->GetMaterials();
	auto& lights = pScene->GetLights();

	const auto aspectRatio = static_cast<float>(m_Width) / static_cast<float>(m_Height);


	for (int px{}; px < m_Width; ++px)
	{
		for (int py{}; py < m_Height; ++py)
		{
			//Get the middle of the pixel
			const auto pcx{ static_cast<float>(px) + 0.5f };
			const auto pcy{ static_cast<float>(py) + 0.5f };

			//From pixel to raster space
			const auto cx = (2.f * pcx / static_cast<float>(m_Width) - 1) * aspectRatio;
			const auto cy = 1 - 2.f * pcy / static_cast<float>(m_Height);

			//From raster to camera space
			const auto ray = Vector3{ cx,cy,1 };
			
			//From camera to world space.
			const Vector3 rayDirection{ ray.Normalized() };

			const Ray viewRay = { {0,0,0}, rayDirection };

			
			ColorRGB finalColor{ };
			HitRecord closestHit{ };

			Sphere testSphere{ {0,0,100}, 50, 0 };
			GeometryUtils::HitTest_Sphere(testSphere, viewRay, closestHit);



			if(closestHit.didHit)
			{
				const auto scaled_t = (closestHit.t - 50.f) / 40.f;
				finalColor = ColorRGB{ scaled_t, scaled_t, scaled_t };
				//finalColor = materials[closestHit.materialIndex]->Shade();
			}

			//Update Color in Buffer
			finalColor.MaxToOne();

			m_pBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBuffer->format,
				static_cast<uint8_t>(finalColor.r * 255),
				static_cast<uint8_t>(finalColor.g * 255),
				static_cast<uint8_t>(finalColor.b * 255));
		}
	}

	//@END
	//Update SDL Surface
	SDL_UpdateWindowSurface(m_pWindow);
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBuffer, "RayTracing_Buffer.bmp");
}
