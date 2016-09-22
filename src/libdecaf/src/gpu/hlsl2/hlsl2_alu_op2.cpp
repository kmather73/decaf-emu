#include "hlsl2_translate.h"
#include "hlsl2_genhelpers.h"

namespace hlsl2
{

void Transpiler::translateAluOp2_ADD(const ControlFlowInst &cf, const AluInstructionGroup &group, SQ_CHAN unit, const AluInst &inst)
{
   auto src0 = genSrcVarStr(cf, group, inst, 0);
   auto src1 = genSrcVarStr(cf, group, inst, 1);

   auto output = fmt::format("{} + {}", src0, src1);
   insertDestAssignStmt(cf, group, unit, inst, output);
}

void Transpiler::translateAluOp2_AND_INT(const ControlFlowInst &cf, const AluInstructionGroup &group, SQ_CHAN unit, const AluInst &inst)
{
   auto src0 = genSrcVarStr(cf, group, inst, 0);
   auto src1 = genSrcVarStr(cf, group, inst, 1);

   auto output = fmt::format("{} & {}", src0, src1);
   insertDestAssignStmt(cf, group, unit, inst, output);
}

void Transpiler::translateAluOp2_ASHR_INT(const ControlFlowInst &cf, const AluInstructionGroup &group, SQ_CHAN unit, const AluInst &inst)
{
   auto src0 = genSrcVarStr(cf, group, inst, 0);
   auto src1 = genSrcVarStr(cf, group, inst, 1);

   auto output = fmt::format("{} >> {}", src0, src1);
   insertDestAssignStmt(cf, group, unit, inst, output);
}

void Transpiler::translateAluOp2_INT_TO_FLT(const ControlFlowInst &cf, const AluInstructionGroup &group, SQ_CHAN unit, const AluInst &inst)
{
   auto src0 = genSrcVarStr(cf, group, inst, 0);

   auto output = fmt::format("(float){}", src0);
   insertDestAssignStmt(cf, group, unit, inst, output);
}

void Transpiler::translateAluOp2_MOV(const ControlFlowInst &cf, const AluInstructionGroup &group, SQ_CHAN unit, const AluInst &inst)
{
   auto src0 = genSrcVarStr(cf, group, inst, 0);

   insertDestAssignStmt(cf, group, unit, inst, src0);
}

void Transpiler::translateAluOp2_MOVA_FLOOR(const ControlFlowInst &cf, const AluInstructionGroup &group, SQ_CHAN unit, const AluInst &inst)
{
   auto src0 = genSrcVarStr(cf, group, inst, 0);

   auto output = fmt::format("clamp((int)floor({}), -256, 255)", src0);
   insertDestAssignStmt(cf, group, unit, inst, output, true);
}

void Transpiler::translateAluOp2_MUL(const ControlFlowInst &cf, const AluInstructionGroup &group, SQ_CHAN unit, const AluInst &inst)
{
   auto src0 = genSrcVarStr(cf, group, inst, 0);
   auto src1 = genSrcVarStr(cf, group, inst, 1);

   auto output = fmt::format("{} * {}", src0, src1);
   insertDestAssignStmt(cf, group, unit, inst, output);
}

void Transpiler::translateAluOp2_MUL_IEEE(const ControlFlowInst &cf, const AluInstructionGroup &group, SQ_CHAN unit, const AluInst &inst)
{
   auto src0 = genSrcVarStr(cf, group, inst, 0);
   auto src1 = genSrcVarStr(cf, group, inst, 1);

   auto output = fmt::format("{} * {}", src0, src1);
   insertDestAssignStmt(cf, group, unit, inst, output);
}

} // namespace hlsl2