#include <windows.h>
#include "gmi2x-cl.hpp"
#include <ImageHlp.h>
#include <stdint.h> 
#include <stdio.h>
#include <stdbool.h>

static void(*Com_Quit_f)() = (void(*)())0x0043a2c0;
extern "C" bool bClosing = false;

void Sys_Unload() {
	bClosing = true;
	static bool unloaded = false;

	if (unloaded)
		return;
	unloaded = true;

	void CL_DiscordShutdown();
	CL_DiscordShutdown();
}

typedef HMODULE(WINAPI* LoadLibraryA_t)(LPCSTR lpLibFileName);
LoadLibraryA_t orig_LoadLibraryA = NULL;


HMODULE WINAPI hLoadLibraryA(LPCSTR lpLibFileName) {
    HMODULE hModule = orig_LoadLibraryA(lpLibFileName);

    if (!hModule) return NULL;

    DWORD pBase = (DWORD)GetModuleHandle(lpLibFileName);
    if (pBase) {
        void Main_UnprotectModule(HMODULE hModule);
        Main_UnprotectModule(hModule);

        if (strstr(lpLibFileName, "uo_cgame_mp") != NULL) {
            void CG_Init(DWORD);
            CG_Init(pBase);
        }
        else if (strstr(lpLibFileName, "uo_game_mp") != NULL) {
            void G_Init(DWORD);
            G_Init(pBase);
        }
    }

    return hModule;
}


// CHATGPT
BYTE original_bytes[5];
BYTE* trampoline = NULL; // dynamically allocated

void patch_opcode_loadlibrary(void)
{
    DWORD oldProtect;
    BYTE* pLoadLibrary = (BYTE*)GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");

    if (!pLoadLibrary)
    {
        printf("Failed to find LoadLibraryA\n");
        return;
    }

    printf("LoadLibraryA address: %p\n", pLoadLibrary);

    // Save original bytes
    memcpy(original_bytes, pLoadLibrary, 5);

    // Allocate trampoline with execute permission
    trampoline = (BYTE*)VirtualAlloc(NULL, 10, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!trampoline)
    {
        printf("Failed to allocate trampoline\n");
        return;
    }

    // Copy original bytes into trampoline
    memcpy(trampoline, original_bytes, 5);

    // Calculate jump back from trampoline+5 to LoadLibraryA+5
    uintptr_t jump_back_addr = (uintptr_t)(pLoadLibrary + 5);
    uintptr_t trampoline_jump_src = (uintptr_t)(trampoline + 5);
    intptr_t rel_jump_back = jump_back_addr - trampoline_jump_src - 5;

    trampoline[5] = 0xE9; // JMP opcode
    *(DWORD*)(trampoline + 6) = (DWORD)rel_jump_back;

    // Set trampoline function pointer
    orig_LoadLibraryA = (LoadLibraryA_t)trampoline;

    // Change protection to writable
    if (!VirtualProtect(pLoadLibrary, 5, PAGE_EXECUTE_READWRITE, &oldProtect))
    {
        printf("VirtualProtect failed\n");
        return;
    }

    // Calculate relative JMP from LoadLibraryA to hLoadLibraryA
    uintptr_t hook_addr = (uintptr_t)hLoadLibraryA;
    uintptr_t loadlib_addr = (uintptr_t)pLoadLibrary;
    intptr_t rel_addr = hook_addr - loadlib_addr - 5;

    pLoadLibrary[0] = 0xE9; // JMP opcode
    *(DWORD*)(pLoadLibrary + 1) = (DWORD)rel_addr;

    // Restore original protection
    if (!VirtualProtect(pLoadLibrary, 5, oldProtect, &oldProtect))
    {
        printf("Failed to restore protection\n");
        return;
    }

    // Flush CPU instruction cache so CPU executes new code
    if (!FlushInstructionCache(GetCurrentProcess(), pLoadLibrary, 5))
    {
        printf("FlushInstructionCache failed\n");
        return;
    }

    printf("Hook installed on LoadLibraryA successfully\n");
}


LONG WINAPI CrashHandler(EXCEPTION_POINTERS* ExceptionInfo)
{
    FILE* log = fopen("gmi2x-cl_log.txt", "w");
    if (log)
    {
        fprintf(log, "GMI2x-Client\n");
        fprintf(log, "Crash caught!\n");
        fprintf(log, "Exception code: 0x%08X\n", ExceptionInfo->ExceptionRecord->ExceptionCode);
        fprintf(log, "Exception address: %p\n", ExceptionInfo->ExceptionRecord->ExceptionAddress);
        fclose(log);
    }
    // Optionally, you could dump more info or trigger a debugger here.

    // Exit process or pass control to the next handler
    return EXCEPTION_EXECUTE_HANDLER;
}
static bool unlock_client_structure() {
    if (!XUNLOCK((void*)cls_realtime, sizeof(int))) return false;
    if (!XUNLOCK((void*)cls_state, sizeof(int))) return false;
    if (!XUNLOCK((void*)clc_demoplaying, 4)) return false;
    if (!XUNLOCK((void*)cls_numglobalservers, sizeof(int))) return false;
    if (!XUNLOCK((void*)cls_pingUpdateSource, sizeof(int))) return false;
    return true;
}

bool applyHooks()
{
	XUNLOCK((void*)0x4f45b0, 1);
	memset((void*)0x4f45b0, 0x00, 1);

	patch_opcode_loadlibrary();
    __call(0x46B565, (int)CleanupExit);

	int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);
	__call(0x56FB98, (int)WinMain);

    unlock_client_structure(); // make some client cls_ structure members writeable etc

    void CL_FrameStub(int msec);
	//void CL_Frame(int msec);
	__call(0x43C8C3, (int)CL_FrameStub/*CL_Frame*/);



	return true;
}


void Main_UnprotectModule(HMODULE hModule)
{
	PIMAGE_DOS_HEADER header = (PIMAGE_DOS_HEADER)hModule;
	PIMAGE_NT_HEADERS ntHeader = (PIMAGE_NT_HEADERS)((DWORD)hModule + header->e_lfanew);

	SIZE_T size = ntHeader->OptionalHeader.SizeOfImage;
	DWORD oldProtect;
	VirtualProtect((LPVOID)hModule, size, PAGE_EXECUTE_READWRITE, &oldProtect);
}

/*DWORD WINAPI DiscordThread(LPVOID lpParam) {
    int startDiscord();
    startDiscord();  // Your existing function
    return 0;
}*/

HINSTANCE hInst;
static int(__stdcall *mainx)(HINSTANCE, HINSTANCE, LPSTR, int) = (int(__stdcall*)(HINSTANCE, HINSTANCE, LPSTR, int))0x46C5C0;
char sys_cmdline[MAX_STRING_CHARS];
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	hInst = hInstance;
	strncpy(sys_cmdline, lpCmdLine, sizeof(sys_cmdline) - 1);

        void CL_DiscordInitialize();
	    CL_DiscordInitialize();  

	return mainx(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
		HMODULE hModule = GetModuleHandle(NULL); // codmp.exe
		if (hModule)
			Main_UnprotectModule(hModule);


	SetUnhandledExceptionFilter(CrashHandler);

/*
#if 0
#ifdef DEBUG
		if (hLogFile == INVALID_HANDLE_VALUE)
		{
			hLogFile = CreateFile(L"./memlog.txt",
				GENERIC_WRITE,
				FILE_SHARE_WRITE, NULL, CREATE_ALWAYS,
				FILE_ATTRIBUTE_NORMAL, NULL);
			_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
			_CrtSetReportFile(_CRT_WARN, hLogFile);
		}
#endif
#endif
*/


		if (!applyHooks())
		{
			MessageBoxA(NULL, "Hooking failed", "GMI2x-Client", MB_OK | MB_ICONERROR);
			Com_Quit_f();
		}

		void _CL_Init();
	    __call(0x0043c166, (int)_CL_Init);
	    __call(0x0043c7c7, (int)_CL_Init);


//	    void CL_DiscordInitialize();
//	    CL_DiscordInitialize();



        //DisableThreadLibraryCalls(hModule); // optional optimization
        //CreateThread(nullptr, 0, CL_DiscordInitialize, nullptr, 0, nullptr);

	}
	break;

	case DLL_PROCESS_DETACH:
	{

	}
	break;

	}
	return TRUE;
}