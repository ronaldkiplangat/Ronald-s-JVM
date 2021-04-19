
#include "stdafx.h"
#include "main.h"
#include "Class.h"
#include "ClassHeap.h"
#include "types.h"
#include "constants.h"
#include "ExecutionEngine.h"

using namespace std;

Class* LoadClass(LPTSTR strClassPath)
{	
	Class *pClass= new Class();
	if(!pClass->LoadClassFromFile(strClassPath))
	{
		delete pClass;
		pClass=NULL;
	}

	return pClass;
}

void Execute(CString strClass)
{
	CString path=strClass;
	ClassHeap heap;
	Class *pClass1, *pClass2, *pClass3;
	pClass1 = new Class();
	pClass2 = new Class();

	BOOL bRet=heap.LoadClass(strClass, pClass1);
	bRet=heap.LoadClass(_T("java/lang/Object"), pClass2);

	ObjectHeap oheap;

	StackFrame *pFrameStack = new StackFrame[20];
	StackFrame::pBaseFrame = pFrameStack;

	memset(pFrameStack, 0, sizeof(Frame)*20);

	StackFrame::pOpStack = new Variable[100];
	memset(StackFrame::pOpStack, 0, sizeof(Variable)*100);

	ExecutionEngine ex;
	ex.pClassHeap = &heap;
	ex.pObjectHeap = &oheap;
	int startFrame=0;

	Object object=oheap.CreateObject(pClass1);
	Class *pVirtualClass=pClass1;
	int mindex=pClass1->GetMethodIndex(_T("Entry"),_T("()I"),pVirtualClass);
	pFrameStack[startFrame].pClass = pVirtualClass;	

	pFrameStack[startFrame].pMethod = &pVirtualClass->methods[mindex];
	pFrameStack[startFrame].stack = StackFrame::pOpStack;
	pFrameStack[startFrame].sp = pFrameStack[startFrame].pMethod->pCode_attr->max_locals;
	pFrameStack[startFrame].stack[0].object = object;
	ex.Execute(&pFrameStack[startFrame]);

}

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;

	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		_tprintf(_T("Fatal Error: MFC initialization failed\n"));
		nRetCode = 1;
	}
	if(argc<2) return -1;

	Execute(argv[1]); 
	
	return nRetCode;
}

