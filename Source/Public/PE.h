#pragma once
#include <iostream>
#include <string>
#include <Windows.h>
#include <vector>
#include "MinHook.h"
#include "Utils.h"

class PE
{
public:
	using EntryPoint = bool(*)(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved);
	PE(const wchar_t* ModuleName);
	PE(HMODULE module) : Module(module) {}

	PIMAGE_NT_HEADERS64 GetNTHeader();
	uintptr_t FindExportedFunction(std::string FuncName);

	HMODULE GetModule() { return Module; };
	bool IsValid() { return Module; };

	bool CallEntryPoint();

	static bool HookFunction(uintptr_t targetFunc, void* newFunc, void* orgFunc);

private:
	void LoadImports();

	bool FixTLS();
	void PatchTLSCalls(PIMAGE_SECTION_HEADER Section);
	void* AllocTLSBlock();

	bool PatchAdress(uintptr_t Address, std::vector<uint8_t> Patch);

	static void LdrInitializeThunkHook(PCONTEXT Context, PVOID NtDllBaseAddress);
	static inline void(*LdrInitializeThunkOrg)(PCONTEXT Context, PVOID NtDllBaseAddress);

	HMODULE Module;
	static inline DWORD TlsSwapIndex;
	static inline DWORD CurrentTlsIndex;
	static inline std::vector<HMODULE> TrackedModules;
	static inline bool DidInit;
};
