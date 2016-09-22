#include "hlsl2_genhelpers.h"
#include "hlsl2_translate.h"

namespace hlsl2
{

void Transpiler::translateTex_SAMPLE(const ControlFlowInst &cf, const TextureFetchInst &inst)
{
   // inst.word0.FETCH_WHOLE_QUAD(); - Optimization which can be ignored
   // inst.word1.LOD_BIAS(); - Only used by certain SAMPLE instructions

   decaf_check(!inst.word0.BC_FRAC_MODE());
   decaf_check(inst.word1.COORD_TYPE_X() == SQ_TEX_COORD_TYPE::NORMALIZED);
   decaf_check(inst.word1.COORD_TYPE_Y() == SQ_TEX_COORD_TYPE::NORMALIZED);
   decaf_check(inst.word1.COORD_TYPE_Z() == SQ_TEX_COORD_TYPE::NORMALIZED);
   decaf_check(inst.word1.COORD_TYPE_W() == SQ_TEX_COORD_TYPE::NORMALIZED);

   auto textureId = inst.word0.RESOURCE_ID();
   auto samplerId = inst.word2.SAMPLER_ID();
   auto texDim = mTexDims[textureId];

   GprMaskRef srcGpr;
   srcGpr.gpr = makeGprRef(inst.word0.SRC_GPR(), inst.word0.SRC_REL(), SQ_INDEX_MODE::LOOP);
   srcGpr.mask[SQ_CHAN::X] = inst.word2.SRC_SEL_X();
   srcGpr.mask[SQ_CHAN::Y] = inst.word2.SRC_SEL_Y();
   srcGpr.mask[SQ_CHAN::Z] = inst.word2.SRC_SEL_Z();
   srcGpr.mask[SQ_CHAN::W] = inst.word2.SRC_SEL_W();

   insertAssignStmt("tmp", genGprRefStr(srcGpr.gpr), srcGpr.mask);

   auto offsetX = inst.word2.OFFSET_X();
   auto offsetY = inst.word2.OFFSET_Y();
   auto offsetZ = inst.word2.OFFSET_Z();
   auto hasOffset = !!offsetX || !!offsetY || !!offsetZ;

   std::string uvStr;
   switch (texDim) {
   case latte::SQ_TEX_DIM::DIM_1D:
      uvStr = "tmp.x";
      break;
   case latte::SQ_TEX_DIM::DIM_1D_ARRAY:
   case latte::SQ_TEX_DIM::DIM_2D:
   case latte::SQ_TEX_DIM::DIM_2D_MSAA:
      uvStr = "tmp.xy";
      break;
   case latte::SQ_TEX_DIM::DIM_3D:
   case latte::SQ_TEX_DIM::DIM_CUBEMAP:
   case latte::SQ_TEX_DIM::DIM_2D_ARRAY:
   case latte::SQ_TEX_DIM::DIM_2D_ARRAY_MSAA:
      uvStr = "tmp.xyz";
      break;
   default:
      decaf_abort("Unexpected texture sample dim");
   }

   std::string offsetStr;
   switch (texDim) {
   case latte::SQ_TEX_DIM::DIM_1D:
   case latte::SQ_TEX_DIM::DIM_1D_ARRAY:
      offsetStr = fmt::format("{}", offsetX);
      break;
   case latte::SQ_TEX_DIM::DIM_2D:
   case latte::SQ_TEX_DIM::DIM_2D_ARRAY:
   case latte::SQ_TEX_DIM::DIM_2D_MSAA:
   case latte::SQ_TEX_DIM::DIM_2D_ARRAY_MSAA:
      offsetStr = fmt::format("int2({}, {})", offsetX, offsetY);
      break;
   case latte::SQ_TEX_DIM::DIM_3D:
      offsetStr = fmt::format("int3({}, {}, {})", offsetX, offsetY, offsetZ);
      break;
   default:
      decaf_abort("Encountered unexpected texture dim for offset");
   }

   insertLineStart();
   mOut.write("tmp = texture{}.Sample(sampler{}, {}, {});", textureId, samplerId, uvStr, offsetStr);
   insertLineEnd();

   GprMaskRef destGpr;
   destGpr.gpr = makeGprRef(inst.word1.DST_GPR(), inst.word1.DST_REL(), SQ_INDEX_MODE::LOOP);
   destGpr.mask[SQ_CHAN::X] = inst.word1.DST_SEL_X();
   destGpr.mask[SQ_CHAN::Y] = inst.word1.DST_SEL_Y();
   destGpr.mask[SQ_CHAN::Z] = inst.word1.DST_SEL_Z();
   destGpr.mask[SQ_CHAN::W] = inst.word1.DST_SEL_W();

   insertAssignStmt(genGprRefStr(destGpr.gpr), "tmp", destGpr.mask);

   auto test = mOut.str();

   mTextureUsed[textureId] = true;
   mSamplerUsed[samplerId] = true;
}

} // namespace hlsl2
