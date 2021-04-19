#include "StdAfx.h"
#include "ExecutionEngine.h"
#include "JavaClass.h"
#include "types.h"
#include "opcodes.h"
#include "ClassHeap.h"
#include "ObjectHeap.h"
#include "NativeMethods.h"

Variable* StackFrame::pOpStack;
StackFrame* StackFrame::pBaseFrame;

ExecutionEngine::~ExecutionEngine(void)
{
}

u4 ExecutionEngine::Execute(StackFrame* pFrameStack)
{
	ASSERT(pFrameStack);
	StackFrame* pFrame=&pFrameStack[0];
	ASSERT(pFrame);

    if(pFrame->pMethod->access_flags & ACC_NATIVE)
	{

		ExecuteNativeMethod(pFrame);

		return 0;
	}

}    