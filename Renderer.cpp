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
#include "Vector3.h"

using namespace dae;

Renderer::Renderer(SDL_Window* pWindow) :
	m_pWindow(pWindow),
	m_pBuffer(SDL_GetWindowSurface(pWindow))
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);
	m_pBufferPixels = static_cast<uint32_t*>(m_pBuffer->pixels);
	m_AspectRatio = static_cast<float>(m_Width) / static_cast<float>(m_Height);

	//Set the lighting mode to the first element
	m_LightingMode = LightingMode::Combined;
}

void Renderer::RenderChunk(int startPx, int endPx, Scene* pScene, const std::vector<Material*>& materials) const
{
	for (int px = startPx; px < endPx; ++px)
	{
		for (int py = 0; py < m_Height; ++py)
		{
			const auto rayDirection = GetRayDirection(static_cast<float>(px), static_cast<float>(py), &pScene->GetCamera());
			const Ray viewRay = { pScene->GetCamera().origin, rayDirection };
			
			ColorRGB finalColor{};
			

			//Contains information about a potential hit.
			HitRecord closestHit{};
			pScene->GetClosestHit(viewRay, closestHit);

			if (closestHit.didHit)
			{
				const Vector3 offsetPosition{ closestHit.origin + closestHit.normal * m_RayOffset };



				for (const auto& light : pScene->GetLights())
				{
					Vector3 lightDirection{ LightUtils::GetDirectionToLight(light, offsetPosition) };
					const auto lightDistance{ lightDirection.Normalize() };


					if (m_ShadowsEnabled)
					{
						const Ray lightRay{ offsetPosition, lightDirection, FLT_MIN, lightDistance };
						HitRecord lightHit{};
						pScene->GetClosestHit(lightRay, lightHit);

						if (lightHit.didHit)
						{
							continue;
						}
					}

					switch (m_LightingMode)
					{
					case LightingMode::ObservedArea: //LambertCosine
					{
						const auto lightNormalAngle{ std::max(Vector3::Dot(closestHit.normal, lightDirection), 0.0f) };
						finalColor += ColorRGB{ lightNormalAngle, lightNormalAngle, lightNormalAngle };
						break;
					}

					case LightingMode::Radiance:
					{
						finalColor += LightUtils::GetRadiance(light, closestHit.origin);
						break;
					}

					case LightingMode::BRDF:
					{
						const ColorRGB BRDF{ materials[closestHit.materialIndex]->Shade(closestHit, lightDirection, -rayDirection) };
						finalColor += BRDF;
						break;
					}

					case LightingMode::Combined:
					{
						const float lightNormalAngle{ std::max(Vector3::Dot(closestHit.normal, lightDirection), 0.0f) };
						const ColorRGB radiance{ LightUtils::GetRadiance(light, closestHit.origin) };
						const ColorRGB BRDF{ materials[closestHit.materialIndex]->Shade(closestHit, lightDirection, -rayDirection) };
						finalColor += radiance * BRDF * lightNormalAngle;
						break;
					}
					}
				}
			}

			//Update Color in Buffer
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
	auto& materials = pScene->GetMaterials();


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

void Renderer::CycleLightingMode()
{
	//Increment the lighting mode
	m_LightingMode = static_cast<LightingMode>((static_cast<int>(m_LightingMode) + 1) % static_cast<int>(LightingMode::COUNT));
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

	//Normalize and return
	return Vector3{ cameraToWorld.TransformVector(ray.Normalized()).Normalized() };
}