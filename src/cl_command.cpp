#include "cl_command.hpp"
#include "client.hpp"


#define PITCH 0
#define YAW 1

void Cmd_LookBack()
{
	float* cl_viewangles = (float*)0x0496379c;
	cl_viewangles[YAW]+= 180;
}


void Cmd_Minimize()
{
	ShowWindow(*gameWindow, SW_MINIMIZE);
}