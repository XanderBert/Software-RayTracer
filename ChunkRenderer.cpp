#include "ChunkRenderer.h"
#include "ChunkRendererManager.h"
#include "Utils.h"

#include "Scene.h"
#include "Vector3.h"
#include "Material.h"
using namespace dae;

void ChunkRenderer::operator()()
{
	RenderChunk();
}

void ChunkRenderer::SetChunk(int startPx, int endPx, Scene* pScene)
{
	m_StartingPixel = startPx;
	m_EndingPixel = endPx;
	m_pScene = pScene;
}

void ChunkRenderer::Render()
{
	m_IsDone = false;
}

void ChunkRenderer::RenderChunk()
{
	const auto& chunkManager = ChunkRendererManager::GetInstance();

	//this will keep the thread alive until the main thread is done.
	while(true)
	{
		if(!m_IsDone)
		{
			for (int px = m_StartingPixel; px < m_EndingPixel; ++px)
			{
				for (int py = 0; py < chunkManager.GetHeight(); ++py)
				{
					const auto rayDirection = GetRayDirection(static_cast<float>(px), static_cast<float>(py));
					const Ray viewRay = { m_pScene->GetCamera().origin, rayDirection };

					ColorRGB finalColor{};


					//Contains information about a potential hit.
					HitRecord closestHit{};
					m_pScene->GetClosestHit(viewRay, closestHit);

					if (closestHit.didHit)
					{
						const Vector3 offsetPosition{ closestHit.origin + closestHit.normal * m_RayOffset };



						for (const auto& light : m_pScene->GetLights())
						{
							Vector3 lightDirection{ LightUtils::GetDirectionToLight(light, offsetPosition) };
							const auto lightDistance{ lightDirection.Normalize() };


							if (ChunkRendererManager::GetInstance().ShadowsEnabled())
							{
								const Ray lightRay{ offsetPosition, lightDirection, FLT_MIN, lightDistance };
								HitRecord lightHit{};
								m_pScene->GetClosestHit(lightRay, lightHit);

								if (lightHit.didHit)
								{
									continue;
								}
							}

							switch (ChunkRendererManager::GetInstance().GetLightingMode())
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
								const ColorRGB BRDF{ m_pScene->GetMaterials()[closestHit.materialIndex]->Shade(closestHit, lightDirection, -rayDirection) };
								finalColor += BRDF;
								break;
							}

							case LightingMode::Combined:
							{
								const float lightNormalAngle{ std::max(Vector3::Dot(closestHit.normal, lightDirection), 0.0f) };
								const ColorRGB radiance{ LightUtils::GetRadiance(light, closestHit.origin) };
								const ColorRGB BRDF{ m_pScene->GetMaterials()[closestHit.materialIndex]->Shade(closestHit, lightDirection, -rayDirection) };
								finalColor += radiance * BRDF * lightNormalAngle;
								break;
							}
							}
						}
					}

					//Update Color in Buffer
					finalColor.MaxToOne();

					chunkManager.GetBufferPixels()[px + (py * chunkManager.GetWidth())] = SDL_MapRGB(chunkManager.GetBuffer()->format,
						static_cast<uint8_t>(finalColor.r * 255),
						static_cast<uint8_t>(finalColor.g * 255),
						static_cast<uint8_t>(finalColor.b * 255));
				}
			}

			m_IsDone = true;
		}
		else
		{
			//thread sleeps for 2 ms 
			std::this_thread::sleep_for(std::chrono::milliseconds(2));
		}
	}

}

Vector3 ChunkRenderer::GetRayDirection(float x, float y) const
{
	const auto& chunkManager = ChunkRendererManager::GetInstance();


	//Get the middle of the pixel
	const auto pcx{ x + 0.5f };
	const auto pcy{ y + 0.5f };

	//From pixel to raster space
	const auto cx = (2.f * pcx / static_cast<float>(chunkManager.GetWidth()) - 1) * chunkManager.GetAspectRatio() * m_pScene->GetCamera().fovAngle;
	const auto cy = 1 - 2.f * pcy / static_cast<float>(chunkManager.GetHeight()) * m_pScene->GetCamera().fovAngle;

	//From raster to camera space
	const auto ray = Vector3{ cx,cy,1 };

	//From camera to world space.
	//Todo: Should be cached in memory
	const Matrix cameraToWorld{ m_pScene->GetCamera().CalculateCameraToWorld() };

	//Normalize and return
	return Vector3{ cameraToWorld.TransformVector(ray.Normalized()).Normalized() };
}
