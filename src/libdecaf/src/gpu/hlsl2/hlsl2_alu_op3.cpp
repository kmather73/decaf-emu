#include "hlsl2_translate.h"
#include "hlsl2_genhelpers.h"

namespace hlsl2
{

void Transpiler::translateAluOp3_MULADD(const ControlFlowInst &cf, const AluInstructionGroup &group, SQ_CHAN unit, const AluInst &inst)
{
   auto src0 = genSrcVarStr(cf, group, inst, 0);
   auto src1 = genSrcVarStr(cf, group, inst, 1);
   auto src2 = genSrcVarStr(cf, group, inst, 2);

   auto output = fmt::format("{} * {} + {}", src0, src1, src2);
   insertDestAssignStmt(cf, group, unit, inst, output);
}

} // namespace hlsl2