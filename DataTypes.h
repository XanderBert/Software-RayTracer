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
			v0{_v0}, v1{_v1}, v2{_v2}, normal{_normal.Normalized()}{ center = (v0 + v1 + v2) / 3.0f;}
		Triangle(const Vector3& _v0, const Vector3& _v1, const Vector3& _v2) :
			v0{ _v0 }, v1{ _v1 }, v2{ _v2 }
		{
			const auto edgeV0V1 = v1 - v0;
			const auto edgeV0V2 = v2 - v0;
			
			//Calculate the normal
			normal = Vector3::Cross(edgeV0V1, edgeV0V2).Normalized();

			//calculate center
			center = (v0 + v1 + v2) / 3.0f;
		}

		Vector3 v0{};
		Vector3 v1{};
		Vector3 v2{};
		Vector3 center{};
		
		Vector3 normal{};

		TriangleCullMode cullMode{};
		unsigned char materialIndex{};
	};

	struct TriangleMesh
	{
		TriangleMesh() = default;

		TriangleMesh(const std::vector<Vector3>& _positions, const std::vector<int>& _indices, TriangleCullMode _cullMode)
				:	indices(_indices)
		,	cullMode(_cullMode)
		{

			positionsX.reserve(_positions.size());
			positionsY.reserve(_positions.size());
			positionsZ.reserve(_positions.size());
			
			for(const auto& position : _positions)
			{
				positionsX.emplace_back(position.x);
				positionsY.emplace_back(position.y);
				positionsZ.emplace_back(position.z);
			}

			
			CalculateNormals();
			UpdateTransforms();
		}

		TriangleMesh(const std::vector<Vector3>& _positions, const std::vector<int>& _indices, const std::vector<Vector3>& _normals, TriangleCullMode _cullMode)
			:
			indices(_indices)
		,	cullMode(_cullMode)
		{
			positionsX.reserve(_positions.size());
			positionsY.reserve(_positions.size());
			positionsZ.reserve(_positions.size());
			
			for(const auto& position : _positions)
			{
				positionsX.emplace_back(position.x);
				positionsY.emplace_back(position.y);
				positionsZ.emplace_back(position.z);
			}


			normalsX.reserve(_normals.size());
			normalsY.reserve(_normals.size());
			normalsZ.reserve(_normals.size());
			for(const auto& normal : _normals)
			{
				normalsX.emplace_back(normal.x);
				normalsY.emplace_back(normal.y);
				normalsZ.emplace_back(normal.z);
			}

			
			UpdateTransforms();
		}

		std::vector<float> positionsX{};
		std::vector<float> positionsY{};
		std::vector<float> positionsZ{};
    
		std::vector<float> normalsX{};
		std::vector<float> normalsY{};
		std::vector<float> normalsZ{};

		std::vector<float> transformedPositionsX{};
		std::vector<float> transformedPositionsY{};
		std::vector<float> transformedPositionsZ{};
    
		std::vector<float> transformedNormalsX{};
		std::vector<float> transformedNormalsY{};
		std::vector<float> transformedNormalsZ{};


		
		std::vector<int> indices{};
		unsigned char materialIndex{};

		TriangleCullMode cullMode{TriangleCullMode::BackFaceCulling};

		Matrix rotationTransform{};
		Matrix translationTransform{};
		Matrix scaleTransform{};

		
		void Translate(const Vector3& translation)
		{
			translationTransform = Matrix::CreateTranslation(translation);
		}

		void RotateY(float yaw)
		{
			rotationTransform = Matrix::CreateRotationY(yaw);
			Matrix::CreateRotationY(yaw);
		}

		void Scale(const Vector3& scale)
		{
			scaleTransform = Matrix::CreateScale(scale);
		}

		void AppendTriangle(const Triangle& triangle, bool ignoreTransformUpdate = false)
		{
			//Get the last index of the last added vertex
			int startIndex = static_cast<int>(positionsX.size());

			//Add each vertex to the positions
			positionsX.emplace_back(triangle.v0.x);
			positionsY.emplace_back(triangle.v0.y);
			positionsZ.emplace_back(triangle.v0.z);

			positionsX.emplace_back(triangle.v1.x);
			positionsY.emplace_back(triangle.v1.y);
			positionsZ.emplace_back(triangle.v1.z);

			positionsX.emplace_back(triangle.v2.x);
			positionsY.emplace_back(triangle.v2.y);
			positionsZ.emplace_back(triangle.v2.z);
			

			
	
			//Add the indices
			indices.emplace_back(startIndex);
			++startIndex;
			indices.emplace_back(startIndex);
			++startIndex;
			indices.emplace_back(startIndex);


			//Add the normals
			normalsX.emplace_back(triangle.normal.x);
			normalsY.emplace_back(triangle.normal.y);
			normalsZ.emplace_back(triangle.normal.z);

			//Set the transforms
			if(!ignoreTransformUpdate) UpdateTransforms();
		}

		void CalculateNormals()
		{
			normalsX.clear();
			normalsY.clear();
			normalsZ.clear();

			const auto amountOfTriangles = GetAmountOfTriangles();
			
			normalsX.reserve(amountOfTriangles);
			normalsY.reserve(amountOfTriangles);
			normalsZ.reserve(amountOfTriangles);
			
			//Go over all triangles
			for (size_t i{}; i < indices.size(); i += 3)
			{
				// Get the position of each vertex in the triangle
				const Vector3 v0{ positionsX[indices[i]], positionsY[indices[i]], positionsZ[indices[i]] };
				const Vector3 v1{ positionsX[indices[i + 1]], positionsY[indices[i + 1]], positionsZ[indices[i + 1]] };
				const Vector3 v2{ positionsX[indices[i + 2]], positionsY[indices[i + 2]], positionsZ[indices[i + 2]] };
				
				//Create 2 edges of the triangle
				const Vector3 edgeV0V1 = v1 - v0;
				const Vector3 edgeV0V2 = v2 - v0;

				//Calculate the normal
				const auto normal = Vector3::Cross(edgeV0V1, edgeV0V2).Normalized();
				
				// Store the individual components of the normal
				normalsX.emplace_back(normal.x);
				normalsY.emplace_back(normal.y);
				normalsZ.emplace_back(normal.z);
			}
		}

		void UpdateTransforms()
		{
			//Create the final transformation matrix
			const auto finalTransformation{ scaleTransform * rotationTransform * translationTransform };
			
			transformedPositionsX.clear();
			transformedPositionsY.clear();
			transformedPositionsZ.clear();

			transformedPositionsX.reserve(positionsX.size());
			transformedPositionsY.reserve(positionsY.size());
			transformedPositionsZ.reserve(positionsZ.size());

			for (size_t i = 0; i < positionsX.size(); ++i)
			{
				const auto& position = Vector3{ positionsX[i], positionsY[i], positionsZ[i] };
				const auto transformedPoint = finalTransformation.TransformPoint(position);
				transformedPositionsX.push_back(transformedPoint.x);
				transformedPositionsY.push_back(transformedPoint.y);
				transformedPositionsZ.push_back(transformedPoint.z);
			}

			transformedNormalsX.clear();
			transformedNormalsY.clear();
			transformedNormalsZ.clear();

			transformedNormalsX.reserve(normalsX.size());
			transformedNormalsY.reserve(normalsY.size());
			transformedNormalsZ.reserve(normalsZ.size());

			for (size_t i = 0; i < normalsX.size(); ++i)
			{
				const auto& normal = Vector3{ normalsX[i], normalsY[i], normalsZ[i] };
				const auto transformedNormal = finalTransformation.TransformVector(normal).Normalized();
				transformedNormalsX.push_back(transformedNormal.x);
				transformedNormalsY.push_back(transformedNormal.y);
				transformedNormalsZ.push_back(transformedNormal.z);
			}
		}

		Triangle GetTriangleByIndex(size_t triangleIndex) const;
		Triangle GetTriangleByVertexIndex(size_t vertexIndex) const;
		size_t GetAmountOfTriangles() const { return (indices.size() / 3); }
		
	};

	inline Triangle TriangleMesh::GetTriangleByIndex(size_t triangleIndex) const
	{
		if(transformedNormalsX.empty() || transformedPositionsX.empty())
		{
			assert(false && "Transformed positions or normals are empty. Did you forget to call UpdateTransforms?");
			return Triangle{};
		}

			
		
		//Check if the index is in range
		if (triangleIndex < (indices.size() / 3))
		{
			const size_t vertexIndex = triangleIndex * 3;
			const Vector3 normal = {transformedNormalsX[triangleIndex], transformedNormalsY[triangleIndex], transformedNormalsZ[triangleIndex]};
			
			auto triangle = Triangle
			{
				{transformedPositionsX[indices[vertexIndex]], transformedPositionsY[indices[vertexIndex]], transformedPositionsZ[indices[vertexIndex]]},
				{transformedPositionsX[indices[vertexIndex + 1]], transformedPositionsY[indices[vertexIndex+ 1]], transformedPositionsZ[indices[vertexIndex+ 1]]},
				{transformedPositionsX[indices[vertexIndex + 2]], transformedPositionsY[indices[vertexIndex + 2]], transformedPositionsZ[indices[vertexIndex + 2]]},
				normal
			};

			triangle.materialIndex = materialIndex;
			triangle.cullMode = cullMode;
			
			return triangle;
		}
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