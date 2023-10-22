#pragma once
#include <SDL_keyboard.h>
#include <SDL_mouse.h>
#include "Math.h"
#include "Timer.h"

namespace dae
{
	struct Camera
	{
		Camera() = default;

		Camera(const Vector3& _origin, float _fovAngle) :
			origin{ _origin },
			fovAngle{ tan(_fovAngle / 2) }
		{
		}


		Vector3 origin{};
		float fovAngle{ 90.f };

		Vector3 forward{ Vector3::UnitZ };
		Vector3 up{ Vector3::UnitY };
		Vector3 right{ Vector3::UnitX };

		float totalPitch{ 0.f };
		float totalYaw{ 0.f };

		Matrix cameraToWorld{};

		static constexpr float velocity{ 2.f };
		inline static Vector3 direction{};


		Matrix CalculateCameraToWorld()
		{
			right = Vector3::Cross(Vector3::UnitY, forward).Normalized();
			up = Vector3::Cross(forward, right).Normalized();


			Matrix worldMatrix{ right,
								up,
								forward,
								origin
			};
			cameraToWorld = worldMatrix;

			return worldMatrix;
		}

		void Update(const Timer* pTimer)
		{
			direction = { Vector3::Zero };
			const float deltaTime = pTimer->GetElapsed();

			// Keyboard Input
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);
			GetKeyboardInput(pKeyboardState, deltaTime);

			// Mouse Input
			int mouseX = 0, mouseY = 0;
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);
			const Vector3 mouseF = { static_cast<float>(mouseX), static_cast<float>(mouseY), 0.0f };

			HandleMouseInput(mouseState, mouseF, deltaTime);

			const Matrix rotationMatrix = CreateRotationMatrix();

			origin += direction;
			forward = rotationMatrix.TransformVector(Vector3::UnitZ);
			forward.Normalize();
		}

		void GetKeyboardInput(const uint8_t* keyboardState, float deltaTime) const
		{
			direction += keyboardState[SDL_SCANCODE_W] ? velocity * forward * deltaTime : Vector3::Zero;
			direction += keyboardState[SDL_SCANCODE_S] ? -velocity * forward * deltaTime : Vector3::Zero;
			direction += keyboardState[SDL_SCANCODE_D] ? velocity * right * deltaTime : Vector3::Zero;
			direction += keyboardState[SDL_SCANCODE_A] ? -velocity * right * deltaTime : Vector3::Zero;
		}

		void HandleMouseInput(uint32_t mouseState, const Vector3& mouseF, float deltaTime)
		{
			switch (mouseState)
			{
			case SDL_BUTTON_LEFT:
				direction -= forward * (mouseF.y * velocity * deltaTime);
				totalYaw -= mouseF.x * velocity * deltaTime;
				break;

			case SDL_BUTTON_RMASK:
				totalYaw -= mouseF.x * velocity * deltaTime;
				totalPitch -= mouseF.y * velocity * deltaTime;
				break;

			case SDL_BUTTON_X2:
				direction -= up * velocity * mouseF.y;
				break;

			default:
				break;
			}
		}

		Matrix CreateRotationMatrix() const
		{
			return Matrix::CreateRotationX(totalPitch) * Matrix::CreateRotationY(totalYaw);
		}
	};
}
