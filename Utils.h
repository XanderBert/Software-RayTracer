#pragma once
#include <fstream>
#include <iostream>
#include "Math.h"
#include "DataTypes.h"

namespace dae
{
	namespace GeometryUtils
	{
#pragma region Sphere HitTest
		//SPHERE HIT-TESTS

		//https://iquilezles.org/articles/intersectors/
		//an implementation of the algebraic solution to the ray-sphere intersection problem.
		inline bool HitTest_Sphere(const Sphere& sphere, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			const auto oc = ray.origin - sphere.origin;
			const float b =Vector3::Dot(oc, ray.direction);
			const auto qc  = oc - b * ray.direction;
			float t = sphere.radius * sphere.radius - Vector3::Dot(qc, qc);

			if(t < 0.0) return false;
			
			t  = sqrtf(t);
			
			const float t0 = -b - t;
			const float t1 = -b + t;
			
			const float root = (t0 < t1) ? t0 : t1;
			
			if (root >= ray.min && root <= ray.max)
			{
				if (!ignoreHitRecord && root < hitRecord.t)
				{
					hitRecord.t = root;
					hitRecord.didHit = true;
					hitRecord.origin = ray.origin + root * ray.direction;
					hitRecord.normal = (hitRecord.origin - sphere.origin).Normalized();
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
			const float directionNormal = Vector3::Dot(plane.normal, ray.direction);

			if (fabs(directionNormal) > FLT_EPSILON)
			{
				const Vector3 planeOrigin = plane.origin - ray.origin;
				const float originNormal = Vector3::Dot(planeOrigin, plane.normal);
				const float t = originNormal / directionNormal;

				// Check if t is within the valid range and not behind a previous hit.
				if (t >= ray.min && t <= ray.max)
				{
					if (!ignoreHitRecord && t < hitRecord.t)
					{
						hitRecord.t = t;
						hitRecord.didHit = true;
						hitRecord.materialIndex = plane.materialIndex;
						hitRecord.origin = ray.origin + t * ray.direction;
						hitRecord.normal = plane.normal;
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
		//TRIANGLE HIT-TESTS
		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			const auto edge1 = triangle.v1 - triangle.v0;
			const auto edge2 = triangle.v2 - triangle.v0;

			const auto h = Vector3::Cross(ray.direction, edge2);
			const auto determinant = Vector3::Dot(edge1, h);


			//if backface culling is active, and the determinant is negative, return false.
			//When determinant is negative, the triangle is facing the other way.
			if (triangle.cullMode == TriangleCullMode::BackFaceCulling && determinant < 0.0f) return false;
			if(triangle.cullMode == TriangleCullMode::FrontFaceCulling && determinant > 0.0f) return false;


			//If the ray is parallel  with the triangle, return;
			if( fabs(determinant) < FLT_EPSILON ) return false;


			const auto inverseDeterminant = 1.0f / determinant;
			const auto tVector = ray.origin - triangle.v0;

			const float u = inverseDeterminant * Vector3::Dot(tVector, h);

			if (u < 0.0f || u > 1.0f) return false;

			const auto q = Vector3::Cross(tVector, edge1);
			const auto v = inverseDeterminant * Vector3::Dot(ray.direction, q);


			if (v < 0.0 || u + v > 1.0) return false;

			//Calculate where the intersection happens on the line.
			const auto t = inverseDeterminant * Vector3::Dot(edge2, q);

			if (t > ray.min && t < ray.max)
			{
				if (!ignoreHitRecord && t < hitRecord.t)
				{
					hitRecord.t = t;
					hitRecord.didHit = true;
					hitRecord.materialIndex = triangle.materialIndex;
					hitRecord.origin = ray.origin + (t * ray.direction);
					hitRecord.normal = triangle.normal;
				}

				return true;
			}

			return false;
		}

		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Triangle(triangle, ray, temp, true);
		}
#pragma endregion
#pragma region TriangeMesh HitTest
		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			// Current closest hit
			HitRecord tempHit{};
			bool hasHit{};

			for (size_t triangleIdx{}; triangleIdx < mesh.GetAmountOfTriangles(); ++triangleIdx)
			{
				auto triangle = mesh.GetTriangleByIndex(triangleIdx);

				if (!HitTest_Triangle(triangle, ray, tempHit, ignoreHitRecord)) continue;
				if (ignoreHitRecord) return true;
				if (!ignoreHitRecord && tempHit.t < hitRecord.t) hitRecord = tempHit;

				hasHit = true;
			}

			return hasHit;
		}
		

		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_TriangleMesh(mesh, ray, temp, true);
		}
#pragma endregion
	}

	namespace LightUtils
	{
		//Direction from target to light
		inline Vector3 GetDirectionToLight(const Light& light, const Vector3 origin)
		{
			if (light.type == LightType::Directional)
			{
				return{ -light.origin };
			}

			if (light.type == LightType::Point)
			{
				return { light.origin - origin };
			}

			return Vector3::Zero;
		}

		inline ColorRGB GetRadiance(const Light& light, const Vector3& target)
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
				const auto targetToLight{ GetDirectionToLight(light, target) };

				lightEnergy = light.color * light.intensity / targetToLight.SqrMagnitude();
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



		
		static bool ParseOBJ(const std::string& filename, std::vector<Vector3>& positions, std::vector<Vector3>& normals, std::vector<int>& indices)
		{
			std::ifstream file(filename);
			if (!file)
				return false;

			std::string sCommand;
			// start a while iteration ending when the end of file is reached (ios::eof)
			while (!file.eof())
			{
				//read the first word of the string, use the >> operator (istream::operator>>) 
				file >> sCommand;
				//use conditional statements to process the different commands	
				if (sCommand == "#"){}
				if(sCommand == "vn"){}
				if(sCommand == "vt"){}
				else if (sCommand == "v")
				{
					//Vertex
					float x, y, z;
					file >> x >> y >> z;
					positions.push_back({ x, y, z });
				}
				else if (sCommand == "f")
				{
					float i0, i1, i2;
					file >> i0 >> i1 >> i2;

					indices.push_back((int)i0 - 1);
					indices.push_back((int)i1 - 1);
					indices.push_back((int)i2 - 1);
				}
				//read till end of line and ignore all remaining chars
				file.ignore(1000, '\n');

				if (file.eof())
					break;
			}

			//Precompute normals
			for (uint64_t index = 0; index < indices.size(); index += 3)
			{
				uint32_t i0 = indices[index];
				uint32_t i1 = indices[index + 1];
				uint32_t i2 = indices[index + 2];

				Vector3 edgeV0V1 = positions[i1] - positions[i0];
				Vector3 edgeV0V2 = positions[i2] - positions[i0];
				Vector3 normal = Vector3::Cross(edgeV0V1, edgeV0V2);

				if (isnan(normal.x))
				{
					//int k = 0;
				}

				normal.Normalize();
				if (isnan(normal.x))
				{
					//int k = 0;
				}

				normals.push_back(normal);
			}

			return true;
		}

		static bool ParseOBJ(const std::string& filename, std::vector<float>& positionsX, std::vector<float>& positionsY, std::vector<float>& positionsZ, std::vector<float>& normalsX, std::vector<float>& normalsY, std::vector<float>& normalsZ, std::vector<int>& indices)
		{
			std::vector<Vector3> positions;
			std::vector<Vector3> normals;

			const bool didParse = ParseOBJ(filename, positions, normals, indices);
			if (!didParse) return false;



			//Positions
			positionsX.reserve(positions.size());
			positionsY.reserve(positions.size());
			positionsZ.reserve(positions.size());
			for(const auto pos : positions)
			{
				positionsX.emplace_back(pos.x);
				positionsY.emplace_back(pos.y);
				positionsZ.emplace_back(pos.z);
			}


			//Normals
			normalsX.reserve(normals.size());
			normalsY.reserve(normals.size());
			normalsZ.reserve(normals.size());
			for(const auto normal : normals)
			{
				normalsX.emplace_back(normal.x);
				normalsY.emplace_back(normal.y);
				normalsZ.emplace_back(normal.z);
			}

			return true;
		}
		
#pragma warning(pop)
	}
}