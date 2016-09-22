#ifdef DECAF_DX12

#include "common/log.h"
#include "dx12_driver.h"

namespace gpu
{

namespace dx12
{

BufferView
Driver::getDataBuffer(void *data, uint32_t size)
{
   auto uploadBuffer = allocateUploadBuffer(size);

   static const CD3DX12_RANGE EmptyRange = { 0, 0 };
   void *uploadData = nullptr;
   uploadBuffer->Map(0, &EmptyRange, &uploadData);

   memcpy(uploadData, data, size);

   uploadBuffer->Unmap(0, nullptr);

   BufferView out;
   out.object = uploadBuffer;
   out.offset = 0;
   return out;
}

bool
Driver::bindDataBuffer(void *data, uint32_t size, const DescriptorsHandle &handle)
{
   auto view = getDataBuffer(data, size);

   D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc;

   viewDesc.Format = DXGI_FORMAT_UNKNOWN;
   viewDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
   viewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

   viewDesc.Buffer.FirstElement = view.offset;
   viewDesc.Buffer.NumElements = size;
   viewDesc.Buffer.StructureByteStride = 0;
   viewDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

   mDevice->CreateShaderResourceView(view.object, nullptr, handle);

   return true;
}

} // namespace dx12

} // namespace gpu

#endif // DECAF_DX12
