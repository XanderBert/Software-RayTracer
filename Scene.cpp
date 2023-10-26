#include "Scene.h"
#include <iostream>
#include "Utils.h"
#include "Material.h"

namespace dae {

#pragma region Base Scene
	//Initialize Scene with Default Solid Color Material (RED)
	Scene::Scene() :
		m_Materials({ new Material_SolidColor({1,0,0}) })
	{
		m_SphereGeometries.reserve(32);
		m_PlaneGeometries.reserve(32);
		m_TriangleMeshGeometries.reserve(32);
		m_Lights.reserve(32);
	}

	Scene::~Scene()
	{
		for (auto& pMaterial : m_Materials)
		{
			delete pMaterial;
			pMaterial = nullptr;
		}

		m_Materials.clear();
	}

	void dae::Scene::GetClosestHit(const Ray& ray, HitRecord& closestHit)
	{
		for (const auto& sphere : m_SphereGeometries)
		{
			GeometryUtils::HitTest_Sphere(sphere, ray, closestHit);
		}

		for (const auto& plane : m_PlaneGeometries)
		{
			GeometryUtils::HitTest_Plane(plane, ray, closestHit);
		}

		m_BVH.IntersectBVH(ray, 0, closestHit);
		// for(const auto& triangle : m_TriangleMeshGeometries)
		// {
		// 	GeometryUtils::HitTest_TriangleMesh(triangle, ray, closestHit);
		// }

		
	}

	bool Scene::DoesHit(const Ray& ray) const
	{
		for (size_t i{}; i < m_SphereGeometries.size(); ++i)
		{
			if (GeometryUtils::HitTest_Sphere(m_SphereGeometries[i], ray))
			{
				return true;
			}
		}

		for (size_t i{}; i < m_PlaneGeometries.size(); ++i)
		{
			if (GeometryUtils::HitTest_Plane(m_PlaneGeometries[i], ray))
			{
				return true;
			}
		}

		for (size_t i{}; i < m_TriangleMeshGeometries.size(); ++i)
		{
			if (GeometryUtils::HitTest_TriangleMesh(m_TriangleMeshGeometries[i], ray))
			{
				return true;
			}
		}


		return  false;
	}

#pragma region Scene Helpers
	Sphere* Scene::AddSphere(const Vector3& origin, float radius, unsigned char materialIndex)
	{
		Sphere s;
		s.origin = origin;
		s.radius = radius;
		s.materialIndex = materialIndex;

		m_SphereGeometries.emplace_back(s);
		return &m_SphereGeometries.back();
	}

	Plane* Scene::AddPlane(const Vector3& origin, const Vector3& normal, unsigned char materialIndex)
	{
		Plane p;
		p.origin = origin;
		p.normal = normal;
		p.materialIndex = materialIndex;

		m_PlaneGeometries.emplace_back(p);
		return &m_PlaneGeometries.back();
	}

	TriangleMesh* Scene::AddTriangleMesh(TriangleCullMode cullMode, unsigned char materialIndex)
	{
		TriangleMesh m{};
		m.cullMode = cullMode;
		m.materialIndex = materialIndex;

		m_TriangleMeshGeometries.emplace_back(m);
		return &m_TriangleMeshGeometries.back();
	}

	Light* Scene::AddPointLight(const Vector3& origin, float intensity, const ColorRGB& color)
	{
		Light l;
		l.origin = origin;
		l.intensity = intensity;
		l.color = color;
		l.type = LightType::Point;

		m_Lights.emplace_back(l);
		return &m_Lights.back();
	}

	Light* Scene::AddDirectionalLight(const Vector3& direction, float intensity, const ColorRGB& color)
	{
		Light l;
		l.direction = direction;
		l.intensity = intensity;
		l.color = color;
		l.type = LightType::Directional;

		m_Lights.emplace_back(l);
		return &m_Lights.back();
	}

	unsigned char Scene::AddMaterial(Material* pMaterial)
	{
		m_Materials.push_back(pMaterial);
		return static_cast<unsigned char>(m_Materials.size() - 1);
	}
#pragma endregion
#pragma endregion

#pragma region SCENE W1
	void Scene_W1::Initialize()
	{
		//default: Material id0 >> SolidColor Material (RED)
		constexpr unsigned char matId_Solid_Red = 0;
		const unsigned char matId_Solid_Blue = AddMaterial(new Material_SolidColor{ colors::Blue });

		const unsigned char matId_Solid_Yellow = AddMaterial(new Material_SolidColor{ colors::Yellow });
		const unsigned char matId_Solid_Green = AddMaterial(new Material_SolidColor{ colors::Green });
		const unsigned char matId_Solid_Magenta = AddMaterial(new Material_SolidColor{ colors::Magenta });

		//Spheres
		AddSphere({ -25.f, 0.f, 100.f }, 50.f, matId_Solid_Red);
		AddSphere({ 25.f, 0.f, 100.f }, 50.f, matId_Solid_Blue);

		//Plane
		AddPlane({ -75.f, 0.f, 0.f }, { 1.f, 0.f,0.f }, matId_Solid_Green);
		AddPlane({ 75.f, 0.f, 0.f }, { -1.f, 0.f,0.f }, matId_Solid_Green);
		AddPlane({ 0.f, -75.f, 0.f }, { 0.f, 1.f,0.f }, matId_Solid_Yellow);
		AddPlane({ 0.f, 75.f, 0.f }, { 0.f, -1.f,0.f }, matId_Solid_Yellow);
		AddPlane({ 0.f, 0.f, 125.f }, { 0.f, 0.f,-1.f }, matId_Solid_Magenta);

	}

	void Scene_W2::Initialize()
	{
		m_Camera.origin = { 0.f, 3.f, -9.f };
		m_Camera.fovAngle = tan(45.f / 2.f);

		//default: Material id0 >> SolidColor Material (RED)
		constexpr unsigned char matId_Solid_Red = 0;
		const unsigned char matId_Solid_Blue = AddMaterial(new Material_SolidColor{ colors::Blue });

		const unsigned char matId_Solid_Yellow = AddMaterial(new Material_SolidColor{ colors::Yellow });
		const unsigned char matId_Solid_Green = AddMaterial(new Material_SolidColor{ colors::Green });
		const unsigned char matId_Solid_Magenta = AddMaterial(new Material_SolidColor{ colors::Magenta });

		//Spheres
		AddSphere({ -1.75f, 1.f, 0.f }, .75f, matId_Solid_Red);
		AddSphere({ 0.f, 1.f, 0.f }, .75f, matId_Solid_Blue);
		AddSphere({ 1.75f, 1.f, 0.f }, .75f, matId_Solid_Red);
		AddSphere({ -1.75f, 3.f, 0.f }, .75f, matId_Solid_Blue);
		AddSphere({ 0.f, 3.f, 0.f }, .75f, matId_Solid_Red);
		AddSphere({ 1.75f, 3.f, 0.f }, .75f, matId_Solid_Blue);

		//Plane
		AddPlane({ -5.f, 0.f, 0.f }, { 1.f, 0.f,0.f }, matId_Solid_Green);
		AddPlane({ 5.f, 0.f, 0.f }, { -1.f, 0.f,0.f }, matId_Solid_Green);
		AddPlane({ 0.f, 0.f, 0.f }, { 0.f, 1.f,0.f }, matId_Solid_Yellow);
		AddPlane({ 0.f, 10.f, 0.f }, { 0.f, -1.f,0.f }, matId_Solid_Yellow);
		AddPlane({ 0.f, 0.f, 10.f }, { 0.f, 0.f,-1.f }, matId_Solid_Magenta);

		AddPointLight({ 0.f, 5.f, -5.f }, 70.f, colors::White);
	}

	void Scene_W3::Initialize()
	{
		m_Camera.origin = { 0.f, 3.f, -9.f };
		m_Camera.fovAngle = tan(45.f / 2.f);

		const auto matCT_GrayRoughMetal = AddMaterial(new Material_CookTorrence({ 0.972f, 0.960f, 0.915f }, 1.f, 1.f));
		const auto matCT_GrayMediumMetal = AddMaterial(new Material_CookTorrence({ 0.972f, 0.960f, 0.915f }, 1.f, 0.6f));
		const auto matCT_GraySmoothMetal = AddMaterial(new Material_CookTorrence({ 0.972f, 0.960f, 0.915f }, 1.f, 0.1f));
		const auto matCT_GrayRoughPlastic = AddMaterial(new Material_CookTorrence({ 0.75f, 0.95f, 0.95f }, 0.f, 1.f));
		const auto matCT_GrayMediumPlastic = AddMaterial(new Material_CookTorrence({ 0.75f, 0.95f, 0.95f }, 0.f, 0.6f));
		const auto matCT_GraySmoothPlastic = AddMaterial(new Material_CookTorrence({ 0.75f, 0.95f, 0.95f }, 0.f, 0.1f));

		const auto matLambert_GrayBlue = AddMaterial(new Material_Lambert({ 0.49f, 0.57f, 0.57f }, 1.f));

		const auto matLambertPhong1 = AddMaterial(new Material_CookTorrence(colors::Gray, 1.f, 1.f));
		const auto matLambertPhong2 = AddMaterial(new Material_CookTorrence(colors::Gray, 1.f, 0.5f));
		const auto matLambertPhong3 = AddMaterial(new Material_CookTorrence(colors::Gray, 1.f, 0.1f));

		//Spheres
		AddSphere({ -1.75f, 1.f, 0.f }, .75f, matCT_GrayRoughMetal);
		AddSphere({ 0.f, 1.f, 0.f }, .75f, matCT_GrayMediumMetal);
		AddSphere({ 1.75f, 1.f, 0.f }, .75f, matCT_GraySmoothMetal);

		AddSphere({ -1.75f, 1.f, 0.f }, .75f, matLambertPhong1);
		AddSphere({ 0.f, 1.f, 0.f }, .75f, matLambertPhong2);
		AddSphere({ 1.75f, 1.f, 0.f }, .75f, matLambertPhong3);

		AddSphere({ -1.75f, 3.f, 0.f }, .75f, matCT_GrayRoughPlastic);
		AddSphere({ 0.f, 3.f, 0.f }, .75f, matCT_GrayMediumPlastic);
		AddSphere({ 1.75f, 3.f, 0.f }, .75f, matCT_GraySmoothPlastic);

		//Plane
		AddPlane({ 0.f, 0.f, 10.f }, { 0.f, 0.f,-1.f }, matLambert_GrayBlue);		//Back
		AddPlane({ 0.f, 0.f, 0.f }, { 0.f, 1.f,0.f }, matLambert_GrayBlue);		//Bottom
		AddPlane({ 0.f, 10.f, 0.f }, { 0.f, -1.f,0.f }, matLambert_GrayBlue);		//Top
		AddPlane({ 5.f, 0.f, 0.f }, { -1.f, 0.f,0.f }, matLambert_GrayBlue);		//Right
		AddPlane({ -5.f, 0.f, 0.f }, { 1.f, 0.f,0.f }, matLambert_GrayBlue);		//Left

		AddPointLight({ 0.f, 5.f, 5.f }, 50.f, ColorRGB{ 1.f, 0.61f, 0.45f });
		AddPointLight({ -2.5f, 5.f, -5.f }, 70.f, ColorRGB{ 1.f, 0.8f, 0.45f });
		AddPointLight({ 2.5f, 2.5f, -5.f }, 50.f, ColorRGB{ .34f, 0.47f, 0.68f });
	}


	void Scene_W4::Initialize()
	{
		sceneName = "Reference Scene";
		m_Camera.origin = { 0.f, 0.2f, -9.f };
		m_Camera.fovAngle = tan(45.f / 2.f);

		const auto matCT_GrayRoughMetal = AddMaterial(new Material_CookTorrence({ .972f, .960f, .915f }, 1.f, 1.f));
		//const auto matCT_GrayMediumMetal = AddMaterial(new Material_CookTorrence({ .972f, .960f, .915f }, 1.f, .6f));
		const auto matCT_GraySmoothMetal = AddMaterial(new Material_CookTorrence({ .972f, .960f, .915f }, 1.f, .1f));
		GetMaterials()[matCT_GraySmoothMetal]->SetReflectivity(0.1f);
		const auto matCT_GrayRoughPlastic = AddMaterial(new Material_CookTorrence({ .75f, .75f, .75f }, .0f, 1.f));
		const auto matCT_GrayMediumPlastic = AddMaterial(new Material_CookTorrence({ .75f, .75f, .75f }, .0f, .6f));
		const auto matCT_GraySmoothPlastic = AddMaterial(new Material_CookTorrence({ .75f, .75f, .75f }, .0f, .1f));

		const auto matLambert_GrayBlue = AddMaterial(new Material_Lambert({ .49f, 0.57f, 0.57f }, 1.f));
		const auto matLambert_GrayBlue_Reflective = AddMaterial(new Material_Lambert({ .49f, 0.57f, 0.57f }, 1.f));
		GetMaterials()[matLambert_GrayBlue_Reflective]->SetReflectivity(0.01f);
		
		const auto matLambert_White = AddMaterial(new Material_Lambert(colors::White, 1.f));

		
		//const auto matLambertPhong3 = AddMaterial(new Material_CookTorrence(colors::Blue, 1.f, 0.1f));

		AddPlane(Vector3{ 0.f, 0.f, 10.f }, Vector3{ 0.f, 0.f, -1.f }, matLambert_GrayBlue); //BACK
		AddPlane(Vector3{ 0.f, 0.f, 0.f }, Vector3{ 0.f, 1.f, 0.f }, matLambert_GrayBlue_Reflective); //BOTTOM
		AddPlane(Vector3{ 0.f, 10.f, 0.f }, Vector3{ 0.f, -1.f, 0.f }, matLambert_GrayBlue); //TOP
		AddPlane(Vector3{ 5.f, 0.f, 0.f }, Vector3{ -1.f, 0.f, 0.f }, matLambert_GrayBlue); //RIGHT
		AddPlane(Vector3{ -5.f, 0.f, 0.f }, Vector3{ 1.f, 0.f, 0.f }, matLambert_GrayBlue); //LEFT

		AddSphere(Vector3{ -1.75f, 1.f, 0.f }, .75f, matCT_GrayRoughMetal);
		//AddSphere(Vector3{ 0.f, 1.f, 0.f }, .75f, matCT_GrayMediumMetal);
		AddSphere(Vector3{ 1.75f, 1.f, 0.f }, .75f, matCT_GraySmoothMetal);
		AddSphere(Vector3{ -1.75f, 3.f, 0.f }, .75f, matCT_GrayRoughPlastic);
		AddSphere(Vector3{ 0.f, 3.f, 0.f }, .75f, matCT_GrayMediumPlastic);
		AddSphere(Vector3{ 1.75f, 3.f, 0.f }, .75f, matCT_GraySmoothPlastic);

		//CW Winding Order!
		Triangle baseTriangle = { Vector3(-.75f, 1.5f, 0.f), Vector3(.75f, 0.f, 0.f), Vector3(-.75f, 0.f, 0.f) };
		
		baseTriangle.cullMode = TriangleCullMode::NoCulling;
		baseTriangle.materialIndex = matCT_GrayRoughMetal;

		m_Meshes.reserve(3);
		
		m_Meshes.emplace_back(AddTriangleMesh(TriangleCullMode::BackFaceCulling, matLambert_White));
		m_Meshes.back()->AppendTriangle(baseTriangle, true);
		m_Meshes.back()->Translate({ -1.75f,4.5f,0.f });
		m_Meshes.back()->UpdateTransforms();
		
		m_Meshes.emplace_back(AddTriangleMesh(TriangleCullMode::BackFaceCulling, matLambert_White));
		m_Meshes.back()->AppendTriangle(baseTriangle, true);
		m_Meshes.back()->Translate({ 0.f,4.5f,0.f });
		m_Meshes.back()->UpdateTransforms();
		
		m_Meshes.emplace_back(AddTriangleMesh(TriangleCullMode::BackFaceCulling, matLambert_White));
		m_Meshes.back()->AppendTriangle(baseTriangle, true);
		m_Meshes.back()->Translate({ 1.75f,4.5f,0.f });
		m_Meshes.back()->UpdateTransforms();

		AddPointLight(Vector3{ 0.f, 5.f, 5.f }, 50.f, ColorRGB{ 1.f, .61f, .45f }); //Backlight
		AddPointLight(Vector3{ -2.5f, 5.f, -5.f }, 70.f, ColorRGB{ 1.f, .8f, .45f }); //Front Light Left
		AddPointLight(Vector3{ 2.5f, 2.5f, -5.f }, 50.f, ColorRGB{ .34f, .47f, .68f });

		

		
		m_BVH.BuildBVH(m_TriangleMeshGeometries);
	}

	void Scene_W4_Bunny::Initialize()
	{
		sceneName = "Bunny Scene";
		m_Camera.origin = { 0.f, 0.2f, -9.f };
		m_Camera.fovAngle = tan(45.f / 2.f);
		const auto matLambert_White = AddMaterial(new Material_Lambert(colors::White, 1.f));
		const auto matLambert_GrayBlue = AddMaterial(new Material_Lambert({ .49f, 0.57f, 0.57f }, 1.f));
			
		AddPointLight(Vector3{ 0.f, 5.f, 5.f }, 50.f, ColorRGB{ 1.f, .61f, .45f }); //Backlight
		AddPointLight(Vector3{ -2.5f, 5.f, -5.f }, 70.f, ColorRGB{ 1.f, .8f, .45f }); //Front Light Left
		AddPointLight(Vector3{ 2.5f, 2.5f, -5.f }, 50.f, ColorRGB{ .34f, .47f, .68f });
		AddPlane(Vector3{ 0.f, 0.f, 0.f }, Vector3{ 0.f, 1.f, 0.f }, matLambert_GrayBlue); //BOTTOM

		//Bunny
		m_Meshes.reserve(1);
		m_Meshes.emplace_back(AddTriangleMesh(TriangleCullMode::BackFaceCulling, matLambert_White));
		Utils::ParseOBJ("Resources/lowpoly_bunny.obj", m_Meshes[0]->positions, m_Meshes[0]->normals, m_Meshes[0]->indices);
		m_Meshes[0]->Translate({3,0, 0});
		m_Meshes[0]->UpdateTransforms();
		
		// m_Meshes.emplace_back(AddTriangleMesh(TriangleCullMode::BackFaceCulling, matLambert_White));
		// Utils::ParseOBJ("Resources/simple_object.obj", m_Meshes[1]->positions, m_Meshes[1]->normals, m_Meshes[1]->indices);
		// m_Meshes[1]->UpdateTransforms();
		
		m_BVH.BuildBVH(m_TriangleMeshGeometries);
	}

#pragma endregion
}
