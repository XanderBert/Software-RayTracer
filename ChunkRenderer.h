#pragma once


namespace dae
{
	struct Camera;
	class Scene;
	struct Vector3;


	class ChunkRenderer final
	{
	public:
		ChunkRenderer() = default;
		~ChunkRenderer() = default;

		//ChunkRenderer(const ChunkRenderer&) = delete;
		//ChunkRenderer(ChunkRenderer&&) noexcept = delete;
		//ChunkRenderer& operator=(const ChunkRenderer&) = delete;
		//ChunkRenderer& operator=(ChunkRenderer&&) noexcept = delete;


		void operator()();

		bool IsDone() const { return m_IsDone; }
		void SetChunk(int startPx, int endPx, Scene* pScene);

		void Render();
	private:
		void RenderChunk();
		Vector3 GetRayDirection(float x, float y) const;

		int m_StartingPixel;
		int m_EndingPixel;
		Scene* m_pScene{};

		static constexpr float m_RayOffset{ 0.001f };

		bool m_IsDone{ true };
	};
}


