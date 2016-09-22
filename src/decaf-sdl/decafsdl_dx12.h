#pragma once

#ifdef DECAF_DX12

#define WIN32_LEAN_AND_MEAN

#include "decafsdl_graphics.h"
#include "libdecaf/decaf.h"
#include "libdecaf/decaf_dx12.h"
#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <wrl.h>

using namespace Microsoft::WRL;

class DecafSDLDX12 : public DecafSDLGraphics
{
   static const auto SwapBufferCount = 2;

public:
   DecafSDLDX12();
   ~DecafSDLDX12() override;

   bool
   initialise(int width, int height) override;

   void
   shutdown() override;

   void
   windowResized() override;

   void
   drawScanBuffer(float vp[4], ID3D12Resource *tex, uint32_t bufferIdx);

   void
   drawScanBuffers(float tv[4], ID3D12Resource *tvBuffer, float drc[4], ID3D12Resource *drcBuffer);

   void
   renderFrame(float tv[4], float drc[4]) override;

   decaf::GraphicsDriver *
   getDecafDriver() override;

protected:
   void waitForPreviousFrame();

   std::thread mGraphicsThread;
   decaf::DX12Driver *mDecafDriver = nullptr;

   ComPtr<ID3D12Device> mDevice = nullptr;
   ComPtr<ID3D12CommandQueue> mCmdQueue = nullptr;
   ComPtr<IDXGISwapChain3> mSwapChain = nullptr;
   ComPtr<ID3D12DescriptorHeap> mRtvHeap;
   ComPtr<ID3D12DescriptorHeap> mSrvHeap;
   ComPtr<ID3D12Resource> mRenderTargets[SwapBufferCount];
   ComPtr<ID3D12CommandAllocator> mCmdAllocator;
   ComPtr<ID3D12RootSignature> mRootSignature;
   ComPtr<ID3D12PipelineState> mPipelineState;
   ComPtr<ID3D12Resource> mVertexBuffer;
   ComPtr<ID3D12GraphicsCommandList> mCmdList;
   ComPtr<ID3D12Fence> mFence;
   UINT mRtvDescriptorSize;
   UINT mSrvDescriptorSize;
   UINT mFrameIndex;
   HANDLE mFenceEvent;
   UINT64 mFenceValue;

};

#endif // DECAF_DX12
