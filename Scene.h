#pragma once
#include <string>
#include <vector>

#include "BVHNode.h"
#include "Math.h"
#include "DataTypes.h"
#include "Camera.h"

namespace dae
{
	//Forward Declarations
	class Timer;
	class Material;
	struct Plane;
	struct Sphere;
	struct Light;

	//Scene Base Class
	class Scene
	{
	public:
		Scene();
		virtual ~Scene();

		Scene(const Scene&) = delete;
		Scene(Scene&&) noexcept = delete;
		Scene& operator=(const Scene&) = delete;
		Scene& operator=(Scene&&) noexcept = delete;

		virtual void Initialize() = 0;
		virtual void Update(dae::Timer* pTimer)
		{
			m_Camera.Update(pTimer);
		}

		Camera& GetCamera() { return m_Camera; }
		void GetClosestHit(const Ray& ray, HitRecord& closestHit);
		bool DoesHit(const Ray& ray);

		const std::vector<Plane>& GetPlaneGeometries() const { return m_PlaneGeometries; }
		const std::vector<Sphere>& GetSphereGeometries() const { return m_SphereGeometries; }
		const std::vector<Light>& GetLights() const { return m_Lights; }
		const std::vector<Material*> GetMaterials() const { return m_Materials; }

		const std::string GetSceneName() const {return sceneName;}

	protected:
		std::string	sceneName;
		std::vector<Plane> m_PlaneGeometries{};
		std::vector<Sphere> m_SphereGeometries{};
		std::vector<TriangleMesh> m_TriangleMeshGeometries{};


		//Todo, Delete this later after debugging
		std::vector<Triangle> m_TriangleGeometries{};
		
		std::vector<Light> m_Lights{};
		std::vector<Material*> m_Materials{};

		Camera m_Camera{};

		Sphere* AddSphere(const Vector3& origin, float radius, unsigned char materialIndex = 0);
		Plane* AddPlane(const Vector3& origin, const Vector3& normal, unsigned char materialIndex = 0);
		TriangleMesh* AddTriangleMesh(TriangleCullMode cullMode, unsigned char materialIndex = 0);

		Light* AddPointLight(const Vector3& origin, float intensity, const ColorRGB& color);
		Light* AddDirectionalLight(const Vector3& direction, float intensity, const ColorRGB& color);
		unsigned char AddMaterial(Material* pMaterial);
		
		BVH m_BVH{};
	};



	//+++++++++++++++++++++++++++++++++++++++++
	//WEEK 1 Test Scene
	class Scene_W1 final : public Scene
	{
	public:
		Scene_W1() = default;
		~Scene_W1() override = default;

		Scene_W1(const Scene_W1&) = delete;
		Scene_W1(Scene_W1&&) noexcept = delete;
		Scene_W1& operator=(const Scene_W1&) = delete;
		Scene_W1& operator=(Scene_W1&&) noexcept = delete;

		void Initialize() override;
	};


	//+++++++++++++++++++++++++++++++++++++++++
	//WEEK 2 Test Scene
	class Scene_W2 final : public Scene
	{
	public:
		Scene_W2() = default;
		~Scene_W2() override = default;

		Scene_W2(const Scene_W2&) = delete;
		Scene_W2(Scene_W2&&) noexcept = delete;
		Scene_W2& operator=(const Scene_W2&) = delete;
		Scene_W2& operator=(Scene_W2&&) noexcept = delete;

		void Initialize() override;
	};


	//+++++++++++++++++++++++++++++++++++++++++
	//WEEK 3 Test Scene
	class Scene_W3 final : public Scene
	{
	public:
		Scene_W3() = default;
		~Scene_W3() override = default;

		Scene_W3(const Scene_W3&) = delete;
		Scene_W3(Scene_W3&&) noexcept = delete;
		Scene_W3& operator=(const Scene_W3&) = delete;
		Scene_W3& operator=(Scene_W3&&) noexcept = delete;

		void Initialize() override;
	};



	//+++++++++++++++++++++++++++++++++++++++++
	//WEEK 4 Test Scene
	class Scene_W4 final : public Scene
	{
	public:
		Scene_W4() = default;
		~Scene_W4() override = default;

		Scene_W4(const Scene_W4&) = delete;
		Scene_W4(Scene_W4&&) noexcept = delete;
		Scene_W4& operator=(const Scene_W4&) = delete;
		Scene_W4& operator=(Scene_W4&&) noexcept = delete;

		void Update(dae::Timer* pTimer) override;

		void Initialize() override;
	private:
		std::vector<TriangleMesh*> m_Meshes{};
	};

	//+++++++++++++++++++++++++++++++++++++++++
	//WEEK 4 Bunny Scene
	class Scene_W4_Bunny final : public Scene
	{
	public:
		Scene_W4_Bunny() = default;
		~Scene_W4_Bunny() override = default;

		Scene_W4_Bunny(const Scene_W4_Bunny&) = delete;
		Scene_W4_Bunny(Scene_W4_Bunny&&) noexcept = delete;
		Scene_W4_Bunny& operator=(const Scene_W4_Bunny&) = delete;
		Scene_W4_Bunny& operator=(Scene_W4_Bunny&&) noexcept = delete;

		void Update(dae::Timer* pTimer) override;
		
		void Initialize() override;
	
	private:
		std::vector<TriangleMesh*> m_Meshes{};
	};
}
