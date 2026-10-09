#include "Public/PE.h"

PE::PE(const wchar_t* ModuleName)
{
	Module = LoadLibraryExW(ModuleName, nullptr, DONT_RESOLVE_DLL_REFERENCES);
    if (!Module) {
        std::cout << "Failed to load Module: " << WStringToString(ModuleName) << "\n";
        return;
    }

    TrackedModules.push_back(Module);

    if (!DidInit)
    {
        TlsSwapIndex = TlsAlloc();
        if (TlsSwapIndex == TLS_OUT_OF_INDEXES) {
            std::cout << "Failed to alloc TLS Index\n";
            return;
        }
        
        auto Module = GetModuleHandleA("ntdll.dll");
        if(!Module) {   
            std::cout << "Failed to get ntdll.dll module\n";
            return;
        }

        auto Proc = GetProcAddress(Module, "LdrInitializeThunk");
        if (!Proc) {
            std::cout << "Failed to get LdrInitializeThunk address\n";
            return;
        }

        MH_Initialize();

        DidInit = HookFunction((uintptr_t)Proc, LdrInitializeThunkHook, &LdrInitializeThunkOrg);
    }

    LoadImports();
    FixTLS();

    std::cout << "FixedUp: " << WStringToString(ModuleName) << "\n";
}

PIMAGE_NT_HEADERS64 PE::GetNTHeader()
{
	if (!Module) return nullptr;

	PIMAGE_DOS_HEADER Dos = (PIMAGE_DOS_HEADER)Module;
	return (PIMAGE_NT_HEADERS64)((uintptr_t)(Module)+Dos->e_lfanew);
}

uintptr_t PE::FindExportedFunction(std::string FuncName)
{
    auto NT = GetNTHeader();
    if (!NT) return 0;

    byte* pModule = (byte*)Module;

    auto ExportDataDir = NT->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
    PIMAGE_EXPORT_DIRECTORY ExportDir = (PIMAGE_EXPORT_DIRECTORY)(pModule + ExportDataDir.VirtualAddress);

    DWORD* NameRvas = (DWORD*)(pModule + ExportDir->AddressOfNames);
    WORD* OrdinalRvas = (WORD*)(pModule + ExportDir->AddressOfNameOrdinals);
    DWORD* FuncRvas = (DWORD*)(pModule + ExportDir->AddressOfFunctions);

    for (DWORD i = 0; i < ExportDir->NumberOfNames; i++) {
        std::string CurrentFuncName = (const char*)(pModule + NameRvas[i]);

        if (CurrentFuncName == FuncName) {
            DWORD FuncRva = FuncRvas[OrdinalRvas[i]];
            return (uintptr_t)(pModule + FuncRva);
        }
    }

    std::cout << "Failed to find export!\n";
    return 0;
}

bool PE::CallEntryPoint()
{   
    auto NT = GetNTHeader();
    if (!NT) return false;

    return EntryPoint((uintptr_t)(Module) + NT->OptionalHeader.AddressOfEntryPoint)(Module, DLL_PROCESS_ATTACH, 0);
}

bool PE::HookFunction(uintptr_t targetFunc, void* newFunc, void* orgFunc)
{
    if (MH_CreateHook((void*)targetFunc, newFunc, reinterpret_cast<void**>(orgFunc)) != MH_OK) {
        std::cout << "Failed to create hook\n";
        return false;
    }

    if (MH_EnableHook((void*)targetFunc) != MH_OK) {
        std::cout << "Failed to enable hook\n";
        return false;
    }

    return true;
}

void PE::LoadImports()
{
	auto NT = GetNTHeader();
	if (!NT) return;

    auto ImportDir = NT->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
    auto ImportDesc = (PIMAGE_IMPORT_DESCRIPTOR)((uintptr_t)(Module) + ImportDir.VirtualAddress);

    for (; ImportDesc->FirstThunk; ImportDesc++)
    {   
        char* LibraryName = (char*)((uintptr_t)(Module) + ImportDesc->Name);
        bool IsUEFNModule = std::string(LibraryName).starts_with("UnrealEditorFortnite");

        HMODULE Library = LoadLibraryExA(LibraryName, nullptr, IsUEFNModule ? DONT_RESOLVE_DLL_REFERENCES : 0);

        if (!Library)
        {
            std::cout << "Failed to load dll: " << LibraryName << "\n";
            continue;
        }

        PIMAGE_THUNK_DATA thunkData = (PIMAGE_THUNK_DATA)((uintptr_t)(Module) + ImportDesc->FirstThunk);
        for (; thunkData->u1.AddressOfData; thunkData++)
        {
            ULONGLONG data = thunkData->u1.AddressOfData;
            PIMAGE_IMPORT_BY_NAME importByName = (PIMAGE_IMPORT_BY_NAME)((uintptr_t)(Module) + data);

            uint8_t* Proc;

            if (IMAGE_SNAP_BY_ORDINAL(thunkData->u1.AddressOfData))
            {
                Proc = (uint8_t*)GetProcAddress(Library, reinterpret_cast<char*>(thunkData->u1.AddressOfData & 0xFFFF));
            }
            else
            {
                PIMAGE_IMPORT_BY_NAME importByName = (PIMAGE_IMPORT_BY_NAME)((uintptr_t)(Module) + data);
                Proc = (uint8_t*)GetProcAddress(Library, importByName->Name);
            }

            std::vector<uint8_t> Patch;
            Patch.insert(Patch.end(), (uint8_t*)&Proc, (uint8_t*)&Proc + sizeof(uint8_t*));

            if (!PatchAdress((uintptr_t)thunkData, Patch))
            {
                std::cout << "Failed to patch import at: " << thunkData << "proc: " << Proc << "proc name: " << importByName->Name << "\n";
            }
        }
    }
}

bool PE::FixTLS()
{
    auto NT = GetNTHeader();
    if (!NT) return false;

    auto TLSDataDir = NT->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS];
    auto TLSDDir = (PIMAGE_TLS_DIRECTORY)((uintptr_t)(Module) + TLSDataDir.VirtualAddress);

    void* TLSBlock = AllocTLSBlock();
    if (!TLSBlock) return false;

    void** NewTlsMap = (void**)realloc(TlsGetValue(TlsSwapIndex), (CurrentTlsIndex + 1) * sizeof(void*));
    if(!NewTlsMap) return false;

    TlsSetValue(TlsSwapIndex, NewTlsMap);

    NewTlsMap[CurrentTlsIndex] = TLSBlock;
    *(DWORD*)TLSDDir->AddressOfIndex = CurrentTlsIndex++;

    auto Section = IMAGE_FIRST_SECTION(NT);
    for (int i = 0; i < NT->FileHeader.NumberOfSections; i++, Section++) {
        if (Section->Characteristics & IMAGE_SCN_MEM_EXECUTE)
            PatchTLSCalls(Section);
    }

    PIMAGE_TLS_CALLBACK* tlsCallbacks = (PIMAGE_TLS_CALLBACK*)(TLSDDir->AddressOfCallBacks);
    while (*tlsCallbacks)
    {
        (*tlsCallbacks)(Module, DLL_PROCESS_ATTACH, nullptr);
        tlsCallbacks++;
    }

    return true;
}

void PE::PatchTLSCalls(PIMAGE_SECTION_HEADER Section)
{   
    DWORD TLSOffset = 0x1480 + TlsSwapIndex * sizeof(void*); // _TEB::TlsSlots = 0x1480

    std::vector<uint8_t> TlsPatch;
    TlsPatch.insert(TlsPatch.end(), (uint8_t*)&TLSOffset, (uint8_t*)&TLSOffset + sizeof(DWORD));

    char* SectionStart = (char*)(uintptr_t(Module) + Section->VirtualAddress);
    for (DWORD i = 0; i < Section->SizeOfRawData; i++) 
    {   
        DWORD* Offset = (DWORD*)&SectionStart[i + 5];

        //mov X, gs:[0x58]
        //65 ? 8B ? 25 58 00 00 00
        if (SectionStart[i + 0] == (char)0x65 &&
            SectionStart[i + 2] == (char)0x8B &&
            SectionStart[i + 4] == (char)0x25 && *Offset == 0x58)
        {   
            PatchAdress((uintptr_t)Offset, TlsPatch);
        }
    }
}

void* PE::AllocTLSBlock()
{
    auto NT = GetNTHeader();
    if (!NT) return nullptr;

    auto TLSDataDir = NT->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS];
    auto TLSDDir = (PIMAGE_TLS_DIRECTORY)((uintptr_t)(Module) + TLSDataDir.VirtualAddress);

    //credits: https://github.com/Fatmike-GH/PELoader/blob/main/PELoader/PELoader/TlsResolver.cpp#L110

    size_t rawDataSize = (BYTE*)TLSDDir->EndAddressOfRawData - (BYTE*)TLSDDir->StartAddressOfRawData;
    size_t totalDataSize = rawDataSize + TLSDDir->SizeOfZeroFill;

    // Allocate memory for the TLS data for the current thread
    LPVOID tlsDataBlock = VirtualAlloc(nullptr, totalDataSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!tlsDataBlock) return nullptr;

    // Copy the default TLS data to the thread specific memory
    if (rawDataSize > 0 && TLSDDir->StartAddressOfRawData != 0)
    {
        // StartAddressOfRawData is a VA, it should be an absolute pointer after relocations.
        memcpy(tlsDataBlock, (void*)TLSDDir->StartAddressOfRawData, rawDataSize);
    }

    if (TLSDDir->SizeOfZeroFill > 0)
    {
        memset((BYTE*)tlsDataBlock + rawDataSize, 0, TLSDDir->SizeOfZeroFill);
    }

    return tlsDataBlock;
}

bool PE::PatchAdress(uintptr_t Address, std::vector<uint8_t> Patch)
{
    DWORD dwProtection;
    if (!VirtualProtect((PVOID)Address, 8, PAGE_EXECUTE_READWRITE, &dwProtection))
        return false;

    for (uint8_t Byte : Patch)
    {
        *(uint8_t*)(Address++) = Byte;
    }

    DWORD dwTemp;
    VirtualProtect((PVOID)Address, 8, dwProtection, &dwTemp);
    return true;
}

void PE::LdrInitializeThunkHook(PCONTEXT Context, PVOID NtDllBaseAddress)
{
    void** NewTlsMap = (void**)malloc(TrackedModules.size() * sizeof(void*));
    if(!NewTlsMap) LdrInitializeThunkOrg(Context, NtDllBaseAddress);

    for (int i = 0; i < TrackedModules.size(); i++) 
    {
        NewTlsMap[i] = PE(TrackedModules[i]).AllocTLSBlock();
    }

    TlsSetValue(TlsSwapIndex, NewTlsMap);

    LdrInitializeThunkOrg(Context, NtDllBaseAddress);
}