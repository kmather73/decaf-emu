#include "hlsl2_translate.h"

namespace hlsl2
{

void Transpiler::translateCf_CALL_FS(const ControlFlowInst &cf)
{
   // We only allow CALL_FS as the very first CF instruction in a vertex shader.
   //  Take a look at translateVtx_SEMANTIC for more details on why this is.
   decaf_check(mCfPC == 0);

   insertLineStart();
   mOut.write("FSMain(input, R);");
   insertLineEnd();
}

void Transpiler::translateCf_RETURN(const ControlFlowInst &cf)
{
   insertLineStart();
   mOut.write("return;");
   insertLineEnd();
}

} // namespace hlsl2