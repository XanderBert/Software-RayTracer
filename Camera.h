#pragma once
#include <cassert>
#include <iostream>
#include <SDL_keyboard.h>
#include <SDL_mouse.h>
#include "Math.h"
#include "Timer.h"

using namespace DirectX;
namespace dae
{
	struct Camera
	{
		Camera() = default;

		Camera(const XMFLOAT3& _origin, float _fovAngle) :
			origin{ _origin },
			fovAngle{ tan(_fovAngle / 2) }
		{
		}


		XMFLOAT3 origin = {};
		float fovAngle{ 90.f };
		
		// Create XMVECTORs for the forward, up, and right directions
		XMFLOAT3 forward = {0,0,1};  // Z-axis points forward
		XMFLOAT3 up = { 0,1,0 };       // Y-axis points up
		XMFLOAT3 right = {1,0,0};    // X-axis points to the right

		float totalPitch{ 0.f };
		float totalYaw{ 0.f };

		XMMATRIX cameraToWorld{};

		float velocity{ 2.f };
		XMFLOAT3 direction{};


		XMMATRIX CalculateCameraToWorld()
		{
			const auto forwardV = XMLoadFloat3(&forward);
			const auto originV = XMLoadFloat3(&origin);

			// Calculate right and up vectors
			XMVECTOR rightV = XMVector3Cross(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), forwardV);  // Y-axis points up
			rightV = XMVector3Normalize(rightV);

			XMVECTOR upV = XMVector3Cross(forwardV, rightV);
			upV = XMVector3Normalize(upV);

			// Create the camera-to-world matrix
			const XMMATRIX worldMatrix(
				rightV,
				upV,
				forwardV,
				originV
			);

			// Transpose the matrix to get the correct camera-to-world transformation
			cameraToWorld = XMMatrixTranspose(worldMatrix);

			//Save the calculated right and up vector
			XMStoreFloat3(&right, rightV);
			XMStoreFloat3(&up, upV);


			return cameraToWorld;
		}

		void Update(const Timer* pTimer)
		{
			const float deltaTime = pTimer->GetElapsed();
			direction = { 0,0,0 };

			const XMVECTOR directionV{ XMLoadFloat3(&direction) };
			XMVECTOR originV{ XMLoadFloat3(&origin) };
			XMVECTOR forwardV = XMLoadFloat3(&forward);



			// Keyboard Input
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);
			GetKeyboardInput(pKeyboardState, deltaTime);

			// Mouse Input
			int mouseX = 0, mouseY = 0;
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

			const XMVECTOR mouseF = XMVectorSet( static_cast<float>(mouseX), static_cast<float>(mouseY), 0.0f,0.0f );

			HandleMouseInput(mouseState, mouseF, deltaTime);

			const XMMATRIX rotationMatrix = CreateRotationMatrix();

			originV += directionV;

			// Transform the forward vector using the rotation matrix
			forwardV = XMVector3TransformCoord(forwardV, rotationMatrix);

			// Normalize the result if needed
			forwardV = XMVector3Normalize(forwardV);


			//Store the results
			XMStoreFloat3(&direction, directionV);
			XMStoreFloat3(&origin, originV);
			XMStoreFloat3(&forward, forwardV);
		}

		void GetKeyboardInput(const uint8_t* keyboardState, float deltaTime)
		{

			
			const auto forwardV = XMLoadFloat3(&forward);
			const auto rightV = XMLoadFloat3(&right);

			const auto forwardSpeed = XMVectorScale(forwardV, velocity * deltaTime);
			const auto rightSpeed = XMVectorScale(rightV, velocity * deltaTime);

			XMVECTOR directionV{XMLoadFloat3(&direction)};

			std::cout << "ForwardSpeed : " <<  (forward.x * velocity * deltaTime) << '\n';
			std::cout << "ForwardSpeed y: " <<  (forward.y * velocity * deltaTime) << '\n';
			std::cout << "ForwardSpeed z: " <<  (forward.z * velocity * deltaTime) << '\n';
			std::cout <<  '\n';
			
			directionV += keyboardState[SDL_SCANCODE_W] ? forwardSpeed : XMVectorZero();
			directionV += keyboardState[SDL_SCANCODE_S] ? -forwardSpeed : XMVectorZero();
			directionV += keyboardState[SDL_SCANCODE_D] ? rightSpeed : XMVectorZero();
			directionV += keyboardState[SDL_SCANCODE_A] ? -rightSpeed : XMVectorZero();


			XMStoreFloat3(&direction, directionV);

			std::cout << "Direction : " << (direction.x * velocity * deltaTime) << '\n';
			std::cout << "Direction y: " << (direction.y * velocity * deltaTime) << '\n';
			std::cout << "Direction z: " << (direction.z * velocity * deltaTime) << '\n';
			std::cout << '\n';
		}

		void HandleMouseInput(uint32_t mouseState, const XMVECTOR& mouseF, float deltaTime)
		{
			const auto x = XMVectorGetX(mouseF);
			const auto y = XMVectorGetY(mouseF);
			const auto forwardV = XMLoadFloat3(&forward);
			const auto upV = XMLoadFloat3(&up);

			XMVECTOR directionV{ XMLoadFloat3(&direction) };
			switch (mouseState)
			{
			case SDL_BUTTON_LEFT:
				directionV -= XMVectorScale(forwardV, (y * velocity * deltaTime));
				totalYaw -= x * velocity * deltaTime;

				break;

			case SDL_BUTTON_RMASK:
				totalYaw -= x * velocity * deltaTime;
				totalPitch -= y * velocity * deltaTime;

				break;

			case SDL_BUTTON_X2:
				directionV -= XMVectorScale(upV, velocity * y);
				break;

			default:
				break;
			}

			XMStoreFloat3(&direction, directionV);
		}

		XMMATRIX CreateRotationMatrix() const
		{
			return XMMatrixRotationRollPitchYaw(totalPitch, totalYaw, 0.0f);
		}
	};
}
