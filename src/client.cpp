/*
                                        revCoD
                        This file is a part of GMI2x-Client
                                        6/16/25
*/

#include "client.hpp"
#include "gmi2x-cl.hpp"
#include "cl_command.hpp"
#include "discord.hpp"


cvar_t* cg_drawFPS;
cvar_t* cg_drawFPS_x;
cvar_t* cg_drawFPS_y;
cvar_t* g_bounce;
cvar_t* cg_drawWeaponSelection;
cvar_t* cl_running;
cvar_t* com_cl_running;
cvar_t* cl_discord_rpc;

void _CL_Init(void)
{
	void(*CL_Init)();
	* (int*)(&CL_Init) = 0x00414500;
	CL_Init();

    Cvar_Set("version", va("[GMI2x-CL] CODUO MP 1.51 build %d %s %s win-x86", 10000, __DATE__, __TIME__));

	Cvar_Get("gmi2x-cl", "1", CVAR_USERINFO | CVAR_ROM);

	cg_drawFPS = Cvar_Get("cg_drawFPS", "0", CVAR_ARCHIVE);
	cg_drawFPS_x = Cvar_Get("cg_drawFPS_x", "523", CVAR_ARCHIVE);
	cg_drawFPS_y = Cvar_Get("cg_drawFPS_y", "2", CVAR_ARCHIVE);
	g_bounce = Cvar_Get("g_bounce", "0", CVAR_ARCHIVE);
	cg_drawWeaponSelection = Cvar_Get("cg_drawWeaponSelection", "1", CVAR_ARCHIVE);

    cl_discord_rpc = Cvar_Get("cl_discord_rpc", "1", CVAR_ARCHIVE);
    
	com_cl_running = Cvar_Get("cl_running", "0", CVAR_ROM);
	cl_running = Cvar_FindVar("cl_running");

	Cvar_Set("com_hunkmegs", "512");

	Cmd_AddCommand("lookback", Cmd_LookBack);
	Cmd_AddCommand("minimize", Cmd_Minimize);

	__nop(0x0040ce36, 5); //fixes spam with "MAX_PACKET_USERCMDS" if you have 1000 fps
}


//extern "C" void (__stdcall *pcl_frame)(int msec) = (void (__stdcall *)(int))0x413870;


typedef void(__stdcall *pcl_frame_t)(int);
pcl_frame_t real_pcl_frame = (pcl_frame_t)0x413870;

void CL_FrameStub(int msec)
{
    if (!com_cl_running->integer) return; 
        
    real_pcl_frame(msec);

    CL_DiscordFrame();

}



/*__attribute__((naked)) void CL_FrameStub() {
    __asm__ __volatile__ (
        "push %%eax\n\t"
        "call *%0\n\t"
        "call *%1\n\t"
        "add $4, %%esp\n\t"
        "ret\n\t"
        :
        : "r"(pcl_frame), "r"(CL_Frame)
        : "eax"
    );
}*/