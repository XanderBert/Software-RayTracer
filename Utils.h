#pragma once
#include <cassert>
#include <fstream>
#include <iostream>
#include <xmmintrin.h>
#include "Math.h"
#include "DataTypes.h"


namespace dae
{
	namespace GeometryUtils
	{
#pragma region Sphere HitTest
		//SPHERE HIT-TESTS
		inline bool HitTest_Sphere(const Sphere& sphere, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			const auto rayOrigin = XMLoadFloat3(&ray.origin);
			const auto rayDirection = XMLoadFloat3(&ray.direction);

			const auto hitRecordOrigin = XMLoadFloat3(&hitRecord.origin);
			const auto sphereOrigin = XMLoadFloat3(&sphere.origin);


			const XMVECTOR oc = (rayOrigin - sphereOrigin);

			const float a = XMVectorGetX(XMVector3Dot(rayDirection, rayDirection));
			const float b = 2.0f * XMVectorGetX(XMVector3Dot(rayDirection, oc));
			const auto c = XMVectorGetX(XMVector3Dot(oc, oc)) - sphere.radius * sphere.radius;

			// Calculate discriminant without taking the square root
			const auto discriminant = b * b - 4 * a * c;

			if (discriminant < 0)
			{
				return false;
			}

			// Calculate both roots
			const float t0 = (-b - sqrtf(discriminant)) / (2.0f * a);
			const float t1 = (-b + sqrtf(discriminant)) / (2.0f * a);

			// Choose the smaller positive root (if any)
			const float root = (t0 < t1) ? t0 : t1;

			if (root >= ray.min && root <= ray.max)
			{
				if (!ignoreHitRecord && root < hitRecord.t)
				{
					hitRecord.t = root;
					hitRecord.didHit = true;

					XMStoreFloat3(&hitRecord.origin, rayOrigin + XMVectorScale(rayDirection, root));
					XMStoreFloat3(&hitRecord.normal, XMVector3Normalize(hitRecordOrigin - sphereOrigin));

					hitRecord.materialIndex = sphere.materialIndex;
				}

				return true;
			}

			return false;
		}

		inline bool HitTest_Sphere(const Sphere& sphere, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Sphere(sphere, ray, temp, true);
		}
#pragma endregion
#pragma region Plane HitTest
		//PLANE HIT-TESTS
		inline bool HitTest_Plane(const Plane& plane, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			// Ensure the plane normal is normalized.
			const auto normalizedNormal = XMVector3Normalize(XMLoadFloat3(&plane.normal));
			const auto rayDirection = XMLoadFloat3(&ray.direction);
			const float directionNormal = XMVectorGetX( XMVector3Dot(normalizedNormal, rayDirection));
			const auto rayOrigin = XMLoadFloat3(&ray.origin);
			const auto planeOriginO = XMLoadFloat3(&plane.origin);

			if (fabs(directionNormal) > FLT_EPSILON)
			{
				const auto planeOrigin = planeOriginO - rayOrigin;
				const float originNormal = XMVectorGetX(XMVector3Dot(planeOrigin, normalizedNormal));
				const float t = originNormal / directionNormal;

				// Check if t is within the valid range and not behind a previous hit.
				if (t >= ray.min && t <= ray.max)
				{
					if (!ignoreHitRecord && t < hitRecord.t)
					{
						hitRecord.t = t;
						hitRecord.didHit = true;
						hitRecord.materialIndex = plane.materialIndex;

						XMStoreFloat3(&hitRecord.origin, rayOrigin + XMVectorScale(rayDirection, t));
						XMStoreFloat3(&hitRecord.normal, normalizedNormal);
					}
					return true;
				}
			}

			return false;
		}

		inline bool HitTest_Plane(const Plane& plane, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Plane(plane, ray, temp, true);
		}
#pragma endregion
#pragma region Triangle HitTest
		////TRIANGLE HIT-TESTS
		//inline bool HitTest_Triangle(const Triangle& /*triangle*/, const Ray& /*ray*/, HitRecord& /*hitRecord*/, bool /*ignoreHitRecord*/ = false)
		//{
		//	//todo W5
		//	assert(false && "No Implemented Yet!");
		//	return false;
		//}

		//inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray)
		//{
		//	HitRecord temp{};
		//	return HitTest_Triangle(triangle, ray, temp, true);
		//}
#pragma endregion
#pragma region TriangeMesh HitTest
		//inline bool HitTest_TriangleMesh(const TriangleMesh& /*mesh*/, const Ray& /*ray*/, HitRecord& /*hitRecord*/, bool /*ignoreHitRecord*/ = false)
		//{
		//	//todo W5
		//	assert(false && "No Implemented Yet!");
		//	return false;
		//}

		//inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray)
		//{
		//	HitRecord temp{};
		//	return HitTest_TriangleMesh(mesh, ray, temp, true);
		//}
#pragma endregion
	}

	namespace LightUtils
	{
		//Direction from target to light
		inline XMFLOAT3 GetDirectionToLight(const Light& light, const XMFLOAT3 origin)
		{
			if (light.type == LightType::Directional)
			{
				return{ MathHelper::NegateXMFLOAT3( light.origin )};
			}

			if (light.type == LightType::Point)
			{
				const auto Origin = XMLoadFloat3(&origin);
				const auto LightOrigin = XMLoadFloat3(&light.origin);
				XMFLOAT3 lightO{};
				XMStoreFloat3(&lightO, LightOrigin - Origin);

				return { lightO };
			}

			return XMFLOAT3{0,0,0};
		}

		inline ColorRGB GetRadiance(const Light& light, const XMFLOAT3& target)
		{
			ColorRGB lightEnergy{};

			switch (light.type)
			{
			case LightType::Directional:
			{
				lightEnergy = light.color * light.intensity;
				break;
			}

			case LightType::Point:
			{

					const auto directionToLight = GetDirectionToLight(light, target);
				const auto targetToLight{ XMLoadFloat3(&directionToLight) };
				

				const auto lightIntensity = XMVectorDivide(XMVector3LengthSq(targetToLight), XMVectorReplicate(light.intensity));
				
				XMVECTOR  color{ XMVectorSet(light.color.r, light.color.g, light.color.b, 0.f) };
					const auto multiplicated = XMVectorMultiply(color, lightIntensity);
				lightEnergy = { XMVectorGetX(multiplicated),XMVectorGetY(multiplicated), XMVectorGetZ(multiplicated) };


				break;
			}
			}
			return lightEnergy;
		}
	}

	namespace Utils
	{
		//Just parses vertices and indices
#pragma warning(push)
#pragma warning(disable : 4505) //Warning unreferenced local function
//		static bool ParseOBJ(const std::string& filename, std::vector<Vector3>& /*positions*/, std::vector<Vector3>& /*normals*/, std::vector<int>& indices)
//		{
//			std::ifstream file(filename);
//			if (!file)
//				return false;
//
//			std::string sCommand;
//			// start a while iteration ending when the end of file is reached (ios::eof)
//			while (!file.eof())
//			{
//				//read the first word of the string, use the >> operator (istream::operator>>) 
//				file >> sCommand;
//				//use conditional statements to process the different commands	
//				if (sCommand == "#")
//				{
//					// Ignore Comment
//				}
//				else if (sCommand == "v")
//				{
//					//Vertex
//					float x, y, z;
//					file >> x >> y >> z;
//					//positions.push_back({ x, y, z });
//				}
//				else if (sCommand == "f")
//				{
//					float i0, i1, i2;
//					file >> i0 >> i1 >> i2;
//
//					indices.push_back((int)i0 - 1);
//					indices.push_back((int)i1 - 1);
//					indices.push_back((int)i2 - 1);
//				}
//				//read till end of line and ignore all remaining chars
//				file.ignore(1000, '\n');
//
//				if (file.eof())
//					break;
//			}
//
//			//Precompute normals
//			for (uint64_t index = 0; index < indices.size(); index += 3)
//			{
//			/*	uint32_t i0 = indices[index];
//				uint32_t i1 = indices[index + 1];
//				uint32_t i2 = indices[index + 2];*/
//
//				//Vector3 edgeV0V1 = positions[i1] - positions[i0];
//				//Vector3 edgeV0V2 = positions[i2] - positions[i0];
//				//Vector3 normal = Vector3::Cross(edgeV0V1, edgeV0V2);
//
//				//if (isnan(normal.x))
//				//{
//				//	//int k = 0;
//				//}
//
//				//normal.Normalize();
//				//if (isnan(normal.x))
//				//{
//				//	//int k = 0;
//				//}
//
//				//normals.push_back(normal);
//			}
//
//			return true;
//		}
#pragma warning(pop)
	}
}