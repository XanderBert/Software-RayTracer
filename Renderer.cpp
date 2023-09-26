//External includes
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include "Renderer.h"

#include <functional>
#include <thread>

#include "Math.h"
#include "Matrix.h"
#include "Material.h"
#include "Scene.h"
#include "Utils.h"

using namespace dae;

Renderer::Renderer(SDL_Window* pWindow) :
	m_pWindow(pWindow),
	m_pBuffer(SDL_GetWindowSurface(pWindow))
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);
	m_pBufferPixels = static_cast<uint32_t*>(m_pBuffer->pixels);

	m_AspectRatio = static_cast<float>(m_Width) / static_cast<float>(m_Height);
}

void Renderer::RenderChunk(int startPx, int endPx, Scene* pScene, const std::vector<Material*>& materials) const
{
	for (int px = startPx; px < endPx; ++px)
	{
		for (int py = 0; py < m_Height; ++py)
		{
			const Ray viewRay = { pScene->GetCamera().origin, GetRayDirection(static_cast<float>(px), static_cast<float>(py), &pScene->GetCamera()) };


			HitRecord closestHit{ };

			pScene->GetClosestHit(viewRay, closestHit);

			ColorRGB finalColor{ };
			if (closestHit.didHit)
			{
				finalColor = materials[closestHit.materialIndex]->Shade();

				for (const auto light : pScene->GetLights())
				{
					constexpr auto offset{ 0.001f };
					const Vector3 offsetPosition{ closestHit.origin + closestHit.normal * offset };
					Vector3 lightDirection{ LightUtils::GetDirectionToLight(light, offsetPosition) };
					Ray lightRay{ offsetPosition, lightDirection.Normalized(), FLT_MIN, lightDirection.Magnitude() };
					HitRecord lightHit{};
					pScene->GetClosestHit(lightRay, lightHit);

					if (lightHit.didHit)
					{
						finalColor *= 0.5f;
					}
				}




			}



			// Update Color in Buffer
			finalColor.MaxToOne();

			m_pBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBuffer->format,
				static_cast<uint8_t>(finalColor.r * 255),
				static_cast<uint8_t>(finalColor.g * 255),
				static_cast<uint8_t>(finalColor.b * 255));
		}
	}
}


void Renderer::Render(Scene* pScene) const
{
	//Camera& camera = pScene->GetCamera();
	auto& materials = pScene->GetMaterials();
	//auto& lights = pScene->GetLights();


	const int chunkSize = m_Width / m_ThreadCount;

	std::vector<std::jthread> threads;

	for (int i = 0; i < m_ThreadCount; ++i)
	{
		int startPx = i * chunkSize;
		int endPx = (i == m_ThreadCount - 1) ? m_Width : (i + 1) * chunkSize;

		threads.emplace_back([this, startPx, endPx, pScene, &materials]()
			{
				RenderChunk(startPx, endPx, pScene, materials);
			});
	}

	//@END
	//Update SDL Surface
	SDL_UpdateWindowSurface(m_pWindow);
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBuffer, "RayTracing_Buffer.bmp");
}

Vector3 Renderer::GetRayDirection(float x, float y, Camera* pCamera) const
{
	//Get the middle of the pixel
	const auto pcx{ x + 0.5f };
	const auto pcy{ y + 0.5f };

	//From pixel to raster space
	const auto cx = (2.f * pcx / static_cast<float>(m_Width) - 1) * m_AspectRatio * pCamera->fovAngle;
	const auto cy = 1 - 2.f * pcy / static_cast<float>(m_Height) * pCamera->fovAngle;

	//From raster to camera space
	const auto ray = Vector3{ cx,cy,1 };

	//From camera to world space.
	//Todo: Should be cached in memory
	const Matrix cameraToWorld{ pCamera->CalculateCameraToWorld() };

	return Vector3{ cameraToWorld.TransformVector(ray.Normalized()).Normalized() };
}
