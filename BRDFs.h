#pragma once
#include <cassert>
#include "Math.h"

namespace dae
{
	namespace BRDF
	{
		/**
		 * \param kd Diffuse Reflection Coefficient
		 * \param cd Diffuse Color
		 * \return Lambert Diffuse Color
		 */

		static ColorRGB Lambert(float kd, const ColorRGB& cd)
		{

			return cd * kd / PI;
		}

		static ColorRGB Lambert(const ColorRGB& kd, const ColorRGB& cd)
		{
			return cd * kd / PI;
		}

		/**
		 * \brief
		 * \param ks Specular Reflection Coefficient
		 * \param exp Phong Exponent
		 * \param l Incoming (incident) Light Direction
		 * \param v View Direction
		 * \param n Normal of the Surface
		 * \return Phong Specular Color
		 */
		static ColorRGB Phong(float ks, float exp, const DirectX::XMFLOAT3& l, const DirectX::XMFLOAT3& v, const DirectX::XMFLOAT3& n)
		{
			const auto lightVector{ XMLoadFloat3(&l) };
			const auto normal{ XMLoadFloat3(&n) };
			const auto reflect =  DirectX::XMVector3Reflect(lightVector, normal);
			const auto view = XMLoadFloat3(&v);

			//Get the dot product between the reflected light vector and the view vector
			const auto reflctedViewDot{ DirectX::XMMax(DirectX::XMVectorGetX(DirectX::XMVector3Dot(reflect, view)), 0.0f)};


			const auto phong{ ks * powf(reflctedViewDot, exp) };

			return ColorRGB{ phong, phong, phong };;
		}

		/**
		 * \brief BRDF Fresnel Function >> Schlick
		 * \param h Normalized Halfvector between View and Light directions
		 * \param v Normalized View direction
		 * \param f0 Base reflectivity of a surface based on IOR (Indices Of Refrection), this is different for Dielectrics (Non-Metal) and Conductors (Metal)
		 * \return
		 */
		static ColorRGB FresnelFunction_Schlick(const DirectX::XMFLOAT3& h, const DirectX::XMFLOAT3& v, ColorRGB& f0)
		{
			const auto hVector{ XMLoadFloat3(&h) };
			const auto vVector{ XMLoadFloat3(&v) };
			const auto dot{ DirectX::XMVectorGetX(DirectX::XMVector3Dot(hVector, vVector)) };
			constexpr auto white = ColorRGB{ 1.0f, 1.0f, 1.0f };


			

			return ColorRGB::Lerp(f0, white, powf(1.0f - dot, 5.0f));
		}

		/**
		 * \brief BRDF NormalDistribution >> Trowbridge-Reitz GGX (UE4 implemetation - squared(roughness))
		 * \param n Surface normal
		 * \param h Normalized half vector
		 * \param roughness Roughness of the material
		 * \return BRDF Normal Distribution Term using Trowbridge-Reitz GGX
		 */
		static float NormalDistribution_GGX(const DirectX::XMFLOAT3& n, const DirectX::XMFLOAT3& h, float roughness)
		{
			const float alpha{ Square(roughness) };
			const auto dot = DirectX::XMVectorGetX(DirectX::XMVector3Dot(XMLoadFloat3(&n), XMLoadFloat3(&h)));


			return Square(alpha) / (PI * Square(Square(dot) * (Square(alpha) - 1.0f) + 1.0f));
		}


		/**
		 * \brief BRDF Geometry Function >> Schlick GGX (Direct Lighting + UE4 implementation - squared(roughness))
		 * \param n Normal of the surface
		 * \param v Normalized view direction
		 * \param roughness Roughness of the material
		 * \return BRDF Geometry Term using SchlickGGX
		 */
		static float GeometryFunction_SchlickGGX(const DirectX::XMFLOAT3& n, const DirectX::XMFLOAT3& v, float roughness)
		{
			const float alpha{ Square(roughness) };
			const float k{ Square(alpha + 1.0f) / 8.0f };
			const auto dot = DirectX::XMMax(DirectX::XMVectorGetX(DirectX::XMVector3Dot(XMLoadFloat3(&n), XMLoadFloat3(&v))), 0.0f);

			return dot / (dot * (1.0f - k) + k);
		}

		/**
		 * \brief BRDF Geometry Function >> Smith (Direct Lighting)
		 * \param n Normal of the surface
		 * \param v Normalized view direction
		 * \param l Normalized light direction
		 * \param roughness Roughness of the material
		 * \return BRDF Geometry Term using Smith (> SchlickGGX(n,v,roughness) * SchlickGGX(n,l,roughness))
		 */
		static float GeometryFunction_Smith(const DirectX::XMFLOAT3& n, const DirectX::XMFLOAT3& v, const DirectX::XMFLOAT3& l, float roughness)
		{

			return GeometryFunction_SchlickGGX(n, v, roughness) * GeometryFunction_SchlickGGX(n, l, roughness);
		}

	}
}