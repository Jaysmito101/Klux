#pragma once

#include "Core/Core.hpp"
#include "Core/ThreadPool.hpp"
#include "Math/Math.hpp"

#include "Impl/RendererCommon.hpp"
#include "Impl/VertexShaderWorker.hpp"
#include "Impl/FragmentShaderWorker.hpp"
#include "Impl/FrameClearWorker.hpp"

namespace klux
{

	class KLUX_API Renderer
	{
	public:

		void BeginFrame();
		void EndFrame();

		void BindFramebuffer(RawPtr<IFramebuffer> fbo);
		void BindPipeline(RawPtr<Pipeline> pipeline);
		void Clear(Bool color = true, Bool depth = true);
		void SetViewport(I32 x, I32 y, I32 width, I32 height);

		void DrawIndexed(RawPtr<Buffer> vertexBuffer, RawPtr<Buffer> indexBuffer, U32 indexCount);
		
		inline void SetClearColor(F32 r, F32 g, F32 b, F32 a) { m_ClearColor = { r, g, b, a }; }

		friend class Device;
	private:
		Renderer();
		~Renderer();

		Bool PassTriangleToFragmentShader(ShaderTriangleRef triangle);

	private:
		Bool m_IsInFrame = false;
		math::Vec4 m_ClearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
		RawPtr<IFramebuffer> m_ActiveFramebuffer = nullptr;
		RawPtr<Pipeline> m_ActivePipeline = nullptr;
		std::optional<Viewport> m_ActiveViewport;


		RawPtr<ThreadPool<8, FrameClearWorkerInput, U32>> m_FrameClearThreadPool;
		RawPtr<IJob<FrameClearWorkerInput, U32>> m_FrameClearJob;

		RawPtr<ThreadPool<8, VertexShaderWorkerInput, U32>> m_VertexShaderThreadPool;
		RawPtr<IJob<VertexShaderWorkerInput, U32>> m_VertexShaderJob;

		static constexpr U32 k_FragmentShaderWorkerCountX = 8;
		static constexpr U32 k_FragmentShaderWorkerCountY = 6;
		U32 m_FragmentShaderTileWidth = 0;
		U32 m_FragmentShaderTileHeight = 0;

		RawPtr<ThreadPool<k_FragmentShaderWorkerCountX * k_FragmentShaderWorkerCountY, FragmentShaderWorkerInput, U32>> m_FragmentShaderThreadPool;
		RawPtr<IJob<FragmentShaderWorkerInput, U32>> m_FragmentShaderJob;

		RawPtr<LinearAllocator> m_VertexToFragmentDataAllocator;
	};
}