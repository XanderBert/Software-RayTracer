#pragma once
#include <cassert>
#include "Math.h"
#include "vector"

namespace dae
{
#pragma region GEOMETRY
	struct Sphere
	{
		Vector3 origin{};
		float radius{};
		unsigned char materialIndex{ 0 };
	};

	struct Plane
	{
		unsigned char materialIndex{ 0 };
		Vector3 origin{};
		Vector3 normal{};
	};

	enum class TriangleCullMode
	{
		FrontFaceCulling,
		BackFaceCulling,
		NoCulling
	};

	struct Triangle
	{
		Triangle() = default;
		Triangle(const Vector3& _v0, const Vector3& _v1, const Vector3& _v2, const Vector3& _normal):
			v0{_v0}, v1{_v1}, v2{_v2}, normal{_normal.Normalized()}{}
		Triangle(const Vector3& _v0, const Vector3& _v1, const Vector3& _v2) :
			v0{ _v0 }, v1{ _v1 }, v2{ _v2 }
		{
			const Vector3 edgeV0V1 = v1 - v0;
			const Vector3 edgeV0V2 = v2 - v0;

			//Calculate the normal
			normal = Vector3::Cross(edgeV0V1, edgeV0V2).Normalized();
		}

		Vector3 v0{};
		Vector3 v1{};
		Vector3 v2{};

		Vector3 normal{};

		TriangleCullMode cullMode{};
		unsigned char materialIndex{};
	};

	struct TriangleMesh
	{
		TriangleMesh() = default;

		TriangleMesh(const std::vector<Vector3>& _positions, const std::vector<int>& _indices, TriangleCullMode _cullMode)
		:	positions(_positions)
		,	indices(_indices)
		,	cullMode(_cullMode)
		{
			CalculateNormals();
			UpdateTransforms();
		}

		TriangleMesh(const std::vector<Vector3>& _positions, const std::vector<int>& _indices, const std::vector<Vector3>& _normals, TriangleCullMode _cullMode)
		:	positions(_positions)
		,	normals(_normals)
		,	indices(_indices)
		,	cullMode(_cullMode)
		{
			UpdateTransforms();
		}

		std::vector<Vector3> positions{};
		std::vector<Vector3> normals{};
		std::vector<int> indices{};
		unsigned char materialIndex{};

		TriangleCullMode cullMode{TriangleCullMode::BackFaceCulling};

		Matrix rotationTransform{};
		Matrix translationTransform{};
		Matrix scaleTransform{};

		std::vector<Vector3> transformedPositions{};
		std::vector<Vector3> transformedNormals{};
		
		void Translate(const Vector3& translation)
		{
			translationTransform = Matrix::CreateTranslation(translation);
		}

		void RotateY(float yaw)
		{
			rotationTransform = Matrix::CreateRotationY(yaw);
		}

		void Scale(const Vector3& scale)
		{
			scaleTransform = Matrix::CreateScale(scale);
		}

		void AppendTriangle(const Triangle& triangle, bool ignoreTransformUpdate = false)
		{
			//Get the last index of the last added vertex
			int startIndex = static_cast<int>(positions.size());

			//Add each vertex to the positions
			positions.emplace_back(triangle.v0);
			positions.emplace_back(triangle.v1);
			positions.emplace_back(triangle.v2);

	
			//Add the indices
			indices.emplace_back(startIndex);
			++startIndex;
			indices.emplace_back(startIndex);
			++startIndex;
			indices.emplace_back(startIndex);


			//Add the normal
			normals.emplace_back(triangle.normal);

			//Set the transforms
			if(!ignoreTransformUpdate) UpdateTransforms();
		}

		void CalculateNormals()
		{
			normals.clear();
			normals.reserve(GetAmountOfTriangles());

			//Go over all triangles
			for (size_t i{}; i < indices.size(); i += 3)
			{
				//Get the position of each vertex in the triangle
				const Vector3& v0 = positions[indices[i]];
				const Vector3& v1 = positions[indices[i + 1]];
				const Vector3& v2 = positions[indices[i + 2]];

				//Create 2 edges of the triangle
				const Vector3 edgeV0V1 = v1 - v0;
				const Vector3 edgeV0V2 = v2 - v0;

				//Calculate the normal
				const auto normal = Vector3::Cross(edgeV0V1, edgeV0V2).Normalized();
				
				normals.emplace_back(normal);
			}
		}

		void UpdateTransforms()
		{
			//Create the final transformation matrix
			const auto finalTransformation{ scaleTransform * rotationTransform * translationTransform };
			
			//Add the positions
			transformedPositions.clear();
			transformedPositions.reserve(positions.size());
			for (const auto& position : positions)
			{
				//Transform the point with the final transformation matrix and add it to the transformed positions
				transformedPositions.emplace_back(finalTransformation.TransformPoint(position));
			}


			//Add the normals
			transformedNormals.clear();
			transformedNormals.reserve(normals.size());
			for (const Vector3& normal : normals)
			{
				//Transform the normal with the final transformation matrix and add it to the transformed normals
				transformedNormals.emplace_back(finalTransformation.TransformVector(normal).Normalized());
			}
		}

		Triangle GetTriangleByIndex(size_t triangleIndex) const;
		Triangle GetTriangleByVertexIndex(size_t vertexIndex) const;
		size_t GetAmountOfTriangles() const { return (indices.size() / 3); }
		
	};

	inline Triangle TriangleMesh::GetTriangleByIndex(size_t triangleIndex) const
	{
		//Check if the index is in range
		if (triangleIndex < (indices.size() / 3))
		{
			const size_t vertexIndex = triangleIndex * 3;
			const auto normal = transformedNormals[triangleIndex];
			auto triangle = Triangle
			{
				transformedPositions[indices[vertexIndex]],
				transformedPositions[indices[vertexIndex + 1]],
				transformedPositions[indices[vertexIndex + 2]],
				normal
			};

			triangle.materialIndex = materialIndex;
			triangle.cullMode = cullMode;
			
			return triangle;

		}

		assert(false && "Triangle index out of bounds");
		//throw std::out_of_range("Triangle index is out of range.");
		return Triangle{};
	}

	inline Triangle TriangleMesh::GetTriangleByVertexIndex(size_t vertexIndex) const
	{
			//get the triangle that is inside the vertexRange of that index [0-3]
			const auto triangleIndex = (vertexIndex - (vertexIndex % 3) / 3);
			return GetTriangleByIndex(triangleIndex);
	}

#pragma endregion
#pragma region LIGHT
	enum class LightType
	{
		Point,
		Directional
	};

	struct Light
	{
		Vector3 origin{};
		Vector3 direction{};
		ColorRGB color{};
		float intensity{};

		LightType type{};
	};
#pragma endregion
#pragma region MISC
	struct Ray
	{
		Vector3 origin{};
		Vector3 direction{};

		float min{ 0.0001f };
		float max{ FLT_MAX };
	};

	struct HitRecord
	{
		Vector3 origin{};
		Vector3 normal{};
		float t = FLT_MAX;

		bool didHit{ false };
		unsigned char materialIndex{ 0 };
	};
#pragma endregion
}