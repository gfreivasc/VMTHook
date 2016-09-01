#include <cstdio>
#include "VMTHook.h"

class Sum
{
public:
	virtual int operate(int a, int b) { return a + b; }
};

#ifdef _WIN64
int operate(Sum* that, int a, int b)
#else
int __stdcall operate(int a, int b)
#endif
{
	return a * b;
}

typedef int(__thiscall *operate_t)(Sum*, int, int);
operate_t oOperate;

int main()
{
	int a, b;
	a = b = 4;

	Sum* sum = new Sum();

	printf("Original sum result: %d\n", sum->operate(a, b));

	oOperate = (operate_t)HookMethod((LPVOID)sum, (PVOID)operate, NULL);

	printf("Call from sum object after hook: %d\n", sum->operate(a, b));
	printf("Call saved original method: %d\n", oOperate(sum, a, b));

	system("Pause");
	return 0;

	delete sum;
}