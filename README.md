# Virtual Method Table Hook
Simple VMT hook implementation for Windows x32/x64.

## What it does?
It intercepts virtual method calls, allowing you to change their behaviour and capturing the class.
It also returns a pointer to the original method.

### Syntax
```c++
LPVOID HookMethod(
    _In_ LPVOID         lpVirtualTable,
    _In_ PVOID          pHookMethod,
    _In_opt_ uintptr_t  dwOffset
);
```

### Parameters
#### _lpVirtualTable_ [in]
A pointer to the base address of the virtual table. The Virtual Table is the first addressed region of the class,
therefore it also refers to object base address.

#### _pHookMethod_ [in]
A pointer to the function that should be called in place of the original virtual method.
Some constraints are applied here.

_**x64**_
In case of 64 bit, the replacing function must take the object pointer as first argument.
This is required by the virtual table as it needs to know which implementation of the method is being called.

_**x32**_
Here it is optional wether or not to take the object pointer as a parameter.
This happens because in x32 the methods use the [`__thiscall`](https://msdn.microsoft.com/pt-br/library/ek8tkfbw.aspx) convention,
which keeps the object pointer in ECX register, and the parameters are kept in stack like in `__stdcall` convention, or `__cdecl` in case of `vararg`.

Therefore, if the object pointer is not required, `__stdcall` should fit.
In case you want the object pointer, you can't declare a function that is not a class member method with `__thiscall`.
A way to override this is using [`__fastcall`](https://msdn.microsoft.com/pt-br/library/6xa169sk.aspx) convention,
Which takes the first and second arguments from ECX and EDX registers respectively, the remaining arguments are passed
normally from stack. Simply define the second argument as a DWORD and ignore it.

#### _dwOffset_ [in, opt]
The position of the desired method in virtual table. It's not an indexer,
so the value must be the precise memory offset from the virtual table base to the method.

_i.e._
If you want the third method inside a x64 virtual table, `dwOffset` must be set to `0x10`,
whereas the second one would be `0x08` and the first one `0x00` or `NULL`.

### Return value
The return value is a pointer to the original method.
Can be saved in an explicit `__thiscall` function pointer that recieves an object pointer and the method arguments.
If it's intended to call the original function at some point. Specially from within `pHookMethod`, that will end up
calling it self recursively, making a stack overflow likely to happen.

For the original function to be called within `pHookMethod`, it needs to take the object pointer as a parameter,
given that the return pointer requires it as an argument.

## Examples
### x64
```c++
class Sum
{
public:
    virtual int operate(int a, int b) { return a + b; }
};

typedef int(__thiscall *operate_t)(Sum*, int, int);
operate_t oOperate;

int operate(Sum* that, int a, int b)
{
    printf("Hooked! Original output: %d\n", oOperate(that, a, b));
    return a * b;
}

int main()
{
    Sum *sum = new Sum();
    printf("  4\n+ 5\n---\n%3d\n", sum->operate(4, 5));
	oOperate = (operate_t)HookMethod((LPVOID)sum, (PVOID)operate, NULL);
	printf("  4\n* 5\n---\n%3d\n", sum->operate(4, 5));
    
    system("Pause");
    delete sum;
    return 0;
}
```

Output:
```
  4
+ 5
---
  9
Hooked! Original output: 9
  4
* 5
---
 20
Press any key to continue. . .
```

### x32
```c++
class Sum
{
public:
    virtual int operate(int a, int b) { return a + b; }
};

typedef int(__thiscall *operate_t)(Sum*, int, int);
operate_t oOperate;

int __stdcall operate_no_orig(int a, int b)
{
	printf("Hooked! Original output not accessible.\n");
	return a - b;
}

int __fastcall operate(Sum* that, DWORD _EDX, int a, int b)  // _EDX can be ignored
{
	printf("Hooked! Original output: %d\n", oOperate(that, a, b));
	return a * b;
}

int main()
{
    Sum *sum = new Sum();
    printf("  4\n+ 5\n---\n%3d\n", sum->operate(4, 5));
	oOperate = (operate_t)HookMethod((LPVOID)sum, (PVOID)operate, NULL);
	printf("  4\n* 5\n---\n%3d\n", sum->operate(4, 5));

    HookMethod((LPVOID)sum, (PVOID)operate_no_orig, NULL);
	printf("  4\n- 5\n---\n%3d\n", sum->operate(4, 5));
    
    system("Pause");
    delete sum;
    return 0;
}
```

Output:
```
  4
+ 5
---
  9
Hooked! Original output: 9
  4
* 5
---
 20
Hooked! Original output not accessible.
  4
- 5
---
 -1
Press any key to continue. . .
```

## Credits
Thordin for his vtable-hook class, MarkHC for explanation on call conventions, Geecko for D3D example, gfreivasc (scim4niac).