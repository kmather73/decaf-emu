#pragma once

#include "gpu/gpu_shadertranspiler.h"
#include "gpu/latte_registers_sq.h"
#include <gsl.h>
#include <map>
#include <spdlog/fmt/fmt.h>
#include <vector>

namespace hlsl2
{

struct InputData
{
   enum class IndexMode : uint32_t {
      PerVertex,
      PerInstance,
   };

   uint32_t bufferIndex;
   uint32_t offset;
   uint32_t elemWidth;
   uint32_t elemCount;
   IndexMode indexMode;
   uint32_t divisor;
};

struct Shader
{
   std::string code;
};

struct VertexShader : public Shader
{
   bool callsFs;
   uint32_t cfileUsed;
   std::array<uint32_t, latte::MaxUniformBlocks> cbufferUsed;
   std::array<bool, latte::MaxSamplers> samplerUsed;
   std::array<bool, latte::MaxTextures> textureUsed;
   std::vector<InputData> inputData;
   uint32_t numPosExports;
   uint32_t numParamExports;
   std::map<uint32_t, uint32_t> exportMap;
};

struct GeometryShader : public Shader
{
   uint32_t cfileUsed;
   std::array<uint32_t, latte::MaxUniformBlocks> cbufferUsed;
   std::array<bool, latte::MaxSamplers> samplerUsed;
   std::array<bool, latte::MaxTextures> textureUsed;
};

struct PixelShader : public Shader
{
   uint32_t cfileUsed;
   std::array<uint32_t, latte::MaxUniformBlocks> cbufferUsed;
   std::array<bool, latte::MaxSamplers> samplerUsed;
   std::array<bool, latte::MaxTextures> textureUsed;
};

class Transpiler : public gpu::CLikeShaderTranspiler
{
public:
   void insertAssignStmt(const std::string &destVar, const std::string &srcVar, std::array<SQ_SEL, 4> srcMask);
   void insertDestAssignStmt(const ControlFlowInst &cf, const AluInstructionGroup &group, SQ_CHAN unit, const AluInst &inst, const std::string &source, bool forAr = false);

   void translateTex_SAMPLE(const ControlFlowInst &cf, const TextureFetchInst &inst) override;

   void translateVtx_SEMANTIC(const ControlFlowInst &cf, const VertexFetchInst &inst) override;

   void translateAluOp2_ADD(const ControlFlowInst &cf, const AluInstructionGroup &group, SQ_CHAN unit, const AluInst &inst) override;
   void translateAluOp2_AND_INT(const ControlFlowInst &cf, const AluInstructionGroup &group, SQ_CHAN unit, const AluInst &inst) override;
   void translateAluOp2_ASHR_INT(const ControlFlowInst &cf, const AluInstructionGroup &group, SQ_CHAN unit, const AluInst &inst) override;
   void translateAluOp2_INT_TO_FLT(const ControlFlowInst &cf, const AluInstructionGroup &group, SQ_CHAN unit, const AluInst &inst) override;
   void translateAluOp2_MOV(const ControlFlowInst &cf, const AluInstructionGroup &group, SQ_CHAN unit, const AluInst &inst) override;
   void translateAluOp2_MOVA_FLOOR(const ControlFlowInst &cf, const AluInstructionGroup &group, SQ_CHAN unit, const AluInst &inst) override;
   void translateAluOp2_MUL(const ControlFlowInst &cf, const AluInstructionGroup &group, SQ_CHAN unit, const AluInst &inst) override;
   void translateAluOp2_MUL_IEEE(const ControlFlowInst &cf, const AluInstructionGroup &group, SQ_CHAN unit, const AluInst &inst) override;

   void translateAluOp3_MULADD(const ControlFlowInst &cf, const AluInstructionGroup &group, SQ_CHAN unit, const AluInst &inst) override;

   void translateAluInst(const ControlFlowInst &cf, const AluInstructionGroup &group, SQ_CHAN unit, const AluInst &inst) override;
   void translateAluGroup(const ControlFlowInst &cf, const AluInstructionGroup &group) override;

   void translateGenericExport(const ControlFlowInst &cf);

   void translateCf_CALL_FS(const ControlFlowInst &cf) override;
   void translateCf_RETURN(const ControlFlowInst &cf) override;
   void translateCf_EXP(const ControlFlowInst &cf) override;
   void translateCf_EXP_DONE(const ControlFlowInst &cf) override;

   void translateCfNormalInst(const ControlFlowInst& cf) override;
   void translateCfExportInst(const ControlFlowInst& cf) override;
   void translateCfAluInst(const ControlFlowInst& cf) override;

   void translate() override;
   static bool translate(gpu::ShaderDesc *shaderDesc, Shader *shader);

protected:
   std::array<bool, latte::MaxSamplers> mSamplerUsed = {{ false }};
   std::array<bool, latte::MaxTextures> mTextureUsed = {{ false }};
   std::array<uint32_t, latte::MaxUniformBlocks> mCbufferNumUsed = {{ 0 }};
   uint32_t mCfileNumUsed = 0;
   std::vector<InputData> mInputData;
   std::map<uint32_t, uint32_t> mVsExportsMap;
   uint32_t mNumExportPos = 0;
   uint32_t mNumExportParam = 0;
   uint32_t mNumExportPix = 0;

};

bool
translate(gpu::ShaderDesc *shaderDesc, Shader *shader);

gpu::VertexShaderDesc
cleanDesc(const gpu::VertexShaderDesc &desc, const VertexShader &shader);

gpu::GeometryShaderDesc
cleanDesc(const gpu::GeometryShaderDesc &desc, const GeometryShader &shader);

gpu::PixelShaderDesc
cleanDesc(const gpu::PixelShaderDesc &desc, const PixelShader &shader);

} // namespace hlsl2