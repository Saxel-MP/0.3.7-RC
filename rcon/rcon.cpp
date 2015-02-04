// [ . . . . ]

//----------------

void con_gpci()
{

	/*
	
	
	
	
	*/

}
//----------------


struct ConsoleCommand_s
{
	char CmdName[280];
	DWORD CmdFlags;
	void (*CmdFunc)();
} ConsoleCommands[] = {
	{"echo",		0,	con_echo},
	{"cmdlist",		0,	con_cmdlist},
	{"exit",		0,	con_exit},
	{"kick",		0,	con_kick},
	{"ban",			0,	con_ban},
	{"gmx",			0,	con_gmx},
	//[ ...]
	{"gpci",        0,  con_gpci},
};


void con_cmdlist()
{
	logprintf("Console Commands:");
	for (int i=0; i<ARRAY_SIZE(ConsoleCommands); i++)
	{
		logprintf("  %s%s", ConsoleCommands[i].CmdName, (ConsoleCommands[i].CmdFlags & CON_CMDFLAG_DEBUG)?" (DEBUG)":"");
	}
	logprintf("");
}
