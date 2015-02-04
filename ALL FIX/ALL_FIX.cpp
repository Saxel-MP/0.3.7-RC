//----------------------------------------------------

void ServerMaxPlayersChanged()
{
	int maxplayers = pConsole->GetIntVariable("maxplayers");
	if (maxplayers < 0)
		pConsole->SetIntVariable("maxplayers", 0);
	if (maxplayers > MAX_PLAYERS)
		pConsole->SetIntVariable("maxplayers", MAX_PLAYERS);

	if (pNetGame)
	{
		if (pConsole->GetIntVariable("maxplayers") > MAX_PLAYERS) {
			pConsole->SetIntVariable("maxplayers", MAX_PLAYERS);
		}
		pNetGame->GetRakServer()->SetAllowedPlayers((WORD)pConsole->GetIntVariable("maxplayers"));
	}
}

//----------------------------------------------------

void ServerInstagibChanged()
{
	if (pNetGame) {
		pNetGame->UpdateInstagib();
	}
}

//----------------------------------------------------

void LoadLogFile()
{
	int reload = 0;
	if (pLogFile)
	{
		fclose(pLogFile);
		reload = 1;
	}
	pLogFile = fopen("server_log.txt", "a");
	if (pLogFile)
	{
		logprintf("");
		logprintf("----------");
		if (reload) logprintf("Reloaded log file: \"server_log.txt\".");
		else logprintf("Loaded log file: \"server_log.txt\".");
		logprintf("----------");
	} else {
		logprintf("Failed to load log file: \"server_log.txt\".");
	}
}

#ifdef LINUX

void SignalHandler(int sig)
{
	switch (sig)
	{
	case SIGUSR1:
		{
		LoadLogFile();
		break;
		}
	case SIGUSR2:
		{
		if (pNetGame)
		{
			pNetGame->LoadBanList();
		}
		break;
		}
	case SIGINT:
	case SIGTERM:
		{
			bQuitApp = true;
			break;
		}
	}
}

//----------------------------------------------------
#include <time.h>

long GetTickCount()
{
	tms tm;
	return (times(&tm) * 10);
}

// strlwr is not included with the GNU C lib it seems.
char* strlwr(char* str)
{
	for (size_t i=0; i<strlen(str); i++)
	{
		if ((str[i] >= 'A') && (str[i] <= 'Z'))
		{
			str[i] -= 32;
		}
	}
	return str;
}

#endif	// #ifdef LINUX


//----------------------------------------------------

#include "main.h"
#include "plugins.h"
#include "signer.h"
#include "jenkinshash.h"
#include "pluginkey.h"
#include "console.h"

//---------------------------------------

void logprintf(char* format, ...);

//---------------------------------------
// Some Helpers

extern "C" RakServerInterface* PluginGetRakServer()
{
	if (pNetGame != NULL)
		return pNetGame->GetRakServer();
	else
		return NULL;
}

extern "C" CNetGame* PluginGetNetGame()
{
	return pNetGame;
}

extern "C" CConsole* PluginGetConsole()
{
	return pConsole;
}

extern "C" bool PluginUnloadFilterScript(char* pFileName)
{
	if (pNetGame != NULL)
		return pNetGame->GetFilterScripts()->UnloadOneFilterScript(pFileName);
	else
		return false;
}

extern "C" bool PluginLoadFilterScriptFromMemory(char* pFileName, char* pFileData)
{
	if (pNetGame != NULL)
		return pNetGame->GetFilterScripts()->LoadFilterScriptFromMemory(pFileName, pFileData);
	else
		return false;
}

extern "C" int PluginCallPublicFS(char *szFunctionName)
{
	if (pNetGame != NULL)
		return pNetGame->GetFilterScripts()->CallPublic(szFunctionName);
	else
		return 0;
}

extern "C" int PluginCallPublicGM(char *szFunctionName)
{
	if (pNetGame != NULL && pNetGame->GetGameMode())
		return pNetGame->GetGameMode()->CallPublic(szFunctionName);
	else
		return 0;
}

//---------------------------------------

CPlugins::CPlugins()
{
	// Set up the table of AMX functions to be exported
#if defined AMX_ALIGN || defined AMX_INIT
	m_AMXExports[PLUGIN_AMX_EXPORT_Align16] = (void*)&amx_Align16;
	m_AMXExports[PLUGIN_AMX_EXPORT_Align32] = (void*)&amx_Align32;
#if defined _I64_MAX || defined HAVE_I64
	m_AMXExports[PLUGIN_AMX_EXPORT_Align64] = (void*)&amx_Align64;
#endif
#endif
	m_AMXExports[PLUGIN_AMX_EXPORT_Allot] = (void*)&amx_Allot;
	m_AMXExports[PLUGIN_AMX_EXPORT_Callback] = (void*)&amx_Callback;
	m_AMXExports[PLUGIN_AMX_EXPORT_Cleanup] = (void*)&amx_Cleanup;
	m_AMXExports[PLUGIN_AMX_EXPORT_Clone] = (void*)&amx_Clone;
	m_AMXExports[PLUGIN_AMX_EXPORT_Exec] = (void*)&amx_Exec;
	m_AMXExports[PLUGIN_AMX_EXPORT_FindNative] = (void*)&amx_FindNative;
	m_AMXExports[PLUGIN_AMX_EXPORT_FindPublic] = (void*)&amx_FindPublic;
	m_AMXExports[PLUGIN_AMX_EXPORT_FindPubVar] = (void*)&amx_FindPubVar;
	m_AMXExports[PLUGIN_AMX_EXPORT_FindTagId] = (void*)&amx_FindTagId;
	m_AMXExports[PLUGIN_AMX_EXPORT_Flags] = (void*)&amx_Flags;
	m_AMXExports[PLUGIN_AMX_EXPORT_GetAddr] = (void*)&amx_GetAddr;
	m_AMXExports[PLUGIN_AMX_EXPORT_GetNative] = (void*)&amx_GetNative;
	m_AMXExports[PLUGIN_AMX_EXPORT_GetPublic] = (void*)&amx_GetPublic;
	m_AMXExports[PLUGIN_AMX_EXPORT_GetPubVar] = (void*)&amx_GetPubVar;
	m_AMXExports[PLUGIN_AMX_EXPORT_GetString] = (void*)&amx_GetString;
	m_AMXExports[PLUGIN_AMX_EXPORT_GetTag] = (void*)&amx_GetTag;
	m_AMXExports[PLUGIN_AMX_EXPORT_GetUserData] = (void*)&amx_GetUserData;
	m_AMXExports[PLUGIN_AMX_EXPORT_Init] = (void*)&amx_Init;
	m_AMXExports[PLUGIN_AMX_EXPORT_InitJIT] = (void*)&amx_InitJIT;
	m_AMXExports[PLUGIN_AMX_EXPORT_MemInfo] = (void*)&amx_MemInfo;
	m_AMXExports[PLUGIN_AMX_EXPORT_NameLength] = (void*)&amx_NameLength;
	m_AMXExports[PLUGIN_AMX_EXPORT_NativeInfo] = (void*)&amx_NativeInfo;
	m_AMXExports[PLUGIN_AMX_EXPORT_NumNatives] = (void*)&amx_NumNatives;
	m_AMXExports[PLUGIN_AMX_EXPORT_NumPublics] = (void*)&amx_NumPublics;
	m_AMXExports[PLUGIN_AMX_EXPORT_NumPubVars] = (void*)&amx_NumPubVars;
	m_AMXExports[PLUGIN_AMX_EXPORT_NumTags] = (void*)&amx_NumTags;
	m_AMXExports[PLUGIN_AMX_EXPORT_Push] = (void*)&amx_Push;
	m_AMXExports[PLUGIN_AMX_EXPORT_PushArray] = (void*)&amx_PushArray;
	m_AMXExports[PLUGIN_AMX_EXPORT_PushString] = (void*)&amx_PushString;
	m_AMXExports[PLUGIN_AMX_EXPORT_RaiseError] = (void*)&amx_RaiseError;
	m_AMXExports[PLUGIN_AMX_EXPORT_Register] = (void*)&amx_Register;
	m_AMXExports[PLUGIN_AMX_EXPORT_Release] = (void*)&amx_Release;
	m_AMXExports[PLUGIN_AMX_EXPORT_SetCallback] = (void*)&amx_SetCallback;
	m_AMXExports[PLUGIN_AMX_EXPORT_SetDebugHook] = (void*)&amx_SetDebugHook;
	m_AMXExports[PLUGIN_AMX_EXPORT_SetString] = (void*)&amx_SetString;
	m_AMXExports[PLUGIN_AMX_EXPORT_SetUserData] = (void*)&amx_SetUserData;
	m_AMXExports[PLUGIN_AMX_EXPORT_StrLen] = (void*)&amx_StrLen;
	//m_AMXExports[PLUGIN_AMX_EXPORT_UTF8Check] = (void*)&amx_UTF8Check;
	//m_AMXExports[PLUGIN_AMX_EXPORT_UTF8Get] = (void*)&amx_UTF8Get;
	//m_AMXExports[PLUGIN_AMX_EXPORT_UTF8Len] = (void*)&amx_UTF8Len;
	//m_AMXExports[PLUGIN_AMX_EXPORT_UTF8Put] = (void*)&amx_UTF8Put;

	// Set up table of Plugin exports
	m_PluginData[PLUGIN_DATA_LOGPRINTF] = (void*)&logprintf;
	
	m_PluginData[PLUGIN_DATA_AMX_EXPORTS] = m_AMXExports;
	m_PluginData[PLUGIN_DATA_CALLPUBLIC_FS] = (void*)&PluginCallPublicFS;
	m_PluginData[PLUGIN_DATA_CALLPUBLIC_GM] = (void*)&PluginCallPublicGM;

	// Internals
	m_PluginData[PLUGIN_DATA_NETGAME] = (void*)&PluginGetNetGame;
	m_PluginData[PLUGIN_DATA_CONSOLE] = (void*)&PluginGetConsole;
	m_PluginData[PLUGIN_DATA_RAKSERVER] = (void*)&PluginGetRakServer;
	m_PluginData[PLUGIN_DATA_LOADFSCRIPT] = (void*)&PluginLoadFilterScriptFromMemory;
	m_PluginData[PLUGIN_DATA_UNLOADFSCRIPT] = (void*)&PluginUnloadFilterScript;

}

//---------------------------------------

CPlugins::~CPlugins()
{

	ServerPluginVector::iterator itor;

	for(itor=m_Plugins.begin(); itor!=m_Plugins.end(); itor++) 
	{
		ServerPlugin_s* pSPlugin = *itor;
		if (pSPlugin->Unload)
			pSPlugin->Unload();
		PLUGIN_UNLOAD(pSPlugin->hModule);
		delete pSPlugin;
	}

}

//---------------------------------------

void CPlugins::ConvertFromHex(unsigned char* pbBuffer, char* szData, unsigned int dwMaxLength)
{
	unsigned int i=0, dwTemp=0;
	char szTemp[4] = {0,0,0,0};
	while(szData[i]!=0)
	{
		if ((i/2) >= dwMaxLength)
			break;
		if (szData[i+0]==0 || szData[i+1]==0)
			break;
		szTemp[0] = szData[i+0];
		szTemp[1] = szData[i+1];
		sscanf(szTemp, "%X", &dwTemp);
		pbBuffer[i/2] = dwTemp;
		i+=2;
	}
}

//---------------------------------------

bool CPlugins::VerifyPluginSignature(char* szPluginFilename)
{
	CSHA1 sha1;
	sha1.HashFile(szPluginFilename);
	sha1.Final();
	BYTE* pbHash = sha1.GetHash();

	CSigner signer;
	signer.SetPublicKey((unsigned char*)PLUGIN_PUB_KEY);

	CHAR szFilename[MAX_PATH];
	strcpy(szFilename, szPluginFilename);
	DWORD dwLength = strlen(szFilename);
	for(DWORD i=dwLength-1; i>0; i--) 
	{
		if (szFilename[i] == '.')
		{
			szFilename[i] = 0;
			break;
		}
	}
	strcat(szFilename, ".sig");

	FILE* pFile = fopen(szFilename, "rt");
	if (pFile == NULL)
		return false;

	DWORD dwSignatureStringSize = signer.GetSignatureSize() * 2 + 1;
	CHAR* szSignature = new CHAR[dwSignatureStringSize];
	BYTE* pbSignature = new BYTE[signer.GetSignatureSize()];
	fgets(szSignature, dwSignatureStringSize, pFile);
	fclose(pFile);
	ConvertFromHex(pbSignature, szSignature, signer.GetSignatureSize());

	signer.SetSignature(pbSignature);
	bool bVerified = signer.VerifyHash(pbHash);
			
	delete pbSignature;
	delete szSignature;

	return bVerified;
}

//---------------------------------------

bool CPlugins::IsValidForNoSign(char* szFilename)
{
	char szLCFilename[MAX_PATH];

	int i = 0, j = 0;
	for(i=strlen(szFilename)-1; i>=0; i--) 
	{
		if (szFilename[i] == '.') 
		{
			j=i;
		}
		if (szFilename[i] == '\\' || szFilename[i] == '/') 
		{
			i++;
			break;
		}
	}

	strcpy(szLCFilename, szFilename+i);		// Get part of filename with the \ or / 
	szLCFilename[j-i] = 0;					// Get rid of extension

	char *szNoSign = pConsole->GetStringVariable("nosign");
	char *szLCNoSign = new char[strlen(szNoSign)+1];
	strcpy(szLCNoSign, szNoSign);

	char *szTok = strtok(szLCNoSign, " ");
	while(szTok)
	{
#ifdef LINUX
		if (strcmp(szTok, szLCFilename)==0) 
#else
		if (strcmpi(szTok, szLCFilename)==0) 
#endif
		{
			delete[] szLCNoSign;
			return true;
		}
		szTok = strtok(NULL, " ");
	}

	delete[] szLCNoSign;

	return false;
}

//---------------------------------------
	
BOOL CPlugins::LoadSinglePlugin(char *szPluginPath) 
{
	// Verify the plugin
	/*
	// NO MORE SIGNATURE VERIFICATION
	bool bVerify = VerifyPluginSignature(szPluginPath);
	if (!bVerify)
	{
		if (IsValidForNoSign(szPluginPath)) 
		{
			logprintf("  Loading without valid signature (nosign).");
			bVerify = true;
		}
	}
	if (!bVerify)
	{
		logprintf("  Could not authenticate validity.");
		return FALSE;
	}
	*/

	// Load it
	ServerPlugin_s* pSPlugin;
	pSPlugin = new ServerPlugin_s();
	
	pSPlugin->hModule = PLUGIN_LOAD(szPluginPath);
	if (pSPlugin->hModule == NULL) 
	{
		// Failed to load
		delete pSPlugin;
		return FALSE;
	}

	pSPlugin->Load = (ServerPluginLoad_t)PLUGIN_GETFUNCTION(pSPlugin->hModule, "Load");
	pSPlugin->Unload = (ServerPluginUnload_t)PLUGIN_GETFUNCTION(pSPlugin->hModule, "Unload");
	pSPlugin->Supports = (ServerPluginSupports_t)PLUGIN_GETFUNCTION(pSPlugin->hModule, "Supports");

	if (pSPlugin->Load == NULL || pSPlugin->Supports == NULL) 
	{
		// Not proper architecture!
		logprintf("  Plugin does not conform to architecture.");
		PLUGIN_UNLOAD(pSPlugin->hModule);
		delete pSPlugin;
		return FALSE;
	}

	pSPlugin->dwSupportFlags = (SUPPORTS_FLAGS)pSPlugin->Supports();

	if ((pSPlugin->dwSupportFlags & SUPPORTS_VERSION_MASK) > SUPPORTS_VERSION) 
	{
		// Unsupported version, sorry!
		logprintf("  Unsupported version - This plugin requires version %x.", (pSPlugin->dwSupportFlags & SUPPORTS_VERSION_MASK));
		PLUGIN_UNLOAD(pSPlugin->hModule);
		delete pSPlugin;
		return FALSE;
	}

	if ((pSPlugin->dwSupportFlags & SUPPORTS_AMX_NATIVES) != 0) 
	{
		pSPlugin->AmxLoad = (ServerPluginAmxLoad_t)PLUGIN_GETFUNCTION(pSPlugin->hModule, "AmxLoad");
		pSPlugin->AmxUnload = (ServerPluginAmxUnload_t)PLUGIN_GETFUNCTION(pSPlugin->hModule, "AmxUnload");
	} 
	else 
	{
		pSPlugin->AmxLoad = NULL;
		pSPlugin->AmxUnload = NULL;
	}

	if ((pSPlugin->dwSupportFlags & SUPPORTS_PROCESS_TICK) != 0)
	{
		pSPlugin->ProcessTick = (ServerPluginProcessTick_t)PLUGIN_GETFUNCTION(pSPlugin->hModule, "ProcessTick");
	}
	else
	{
		pSPlugin->ProcessTick = NULL;
	}

	if (!pSPlugin->Load(m_PluginData)) 
	{
		// Initialize failed!
		PLUGIN_UNLOAD(pSPlugin->hModule);
		delete pSPlugin;
		return FALSE;
	}

	m_Plugins.push_back(pSPlugin);

	return TRUE;

}

//---------------------------------------

void CPlugins::LoadPlugins(char *szSearchPath)
{
	char szPath[MAX_PATH];
	char szFullPath[MAX_PATH];
	char *szDlerror = NULL;
	strcpy(szPath, szSearchPath);

#ifdef LINUX
	if (szPath[strlen(szPath)-1] != '/') 
		strcat(szPath, "/");
#else
	if (szPath[strlen(szPath)-1] != '\\') 
		strcat(szPath, "\\");
#endif

	logprintf("");
	logprintf("Server Plugins");
	logprintf("--------------");
	int iPluginCount = 0;
	char* szFilename = strtok(pConsole->GetStringVariable("plugins"), " ");
	while (szFilename)
	{
		logprintf(" Loading plugin: %s", szFilename);

		strcpy(szFullPath, szPath);
		strcat(szFullPath, szFilename);

		if (LoadSinglePlugin(szFullPath))
		{
            logprintf("  Loaded.");
		} 
		else 
		{
#ifdef LINUX
			szDlerror = PLUGIN_GETERROR();
			if (szDlerror) 
				logprintf("  Failed (%s)", szDlerror);
			else 
				logprintf("  Failed.");
#else
			logprintf("  Failed.");
#endif
		}

		szFilename = strtok(NULL, " ");
	}
	logprintf(" Loaded %d plugins.\n", GetPluginCount());
}

// [OBSOLETE: Using the non search defined method]
void CPlugins::LoadPluginsSearch(char *szSearchPath)
{
#ifdef LINUX
	DIR *dir = opendir(szSearchPath);
	struct dirent *file;
	char *szFilename = NULL;
	char *szExt = NULL;
	int iStrLen;
	char szPath[255];
	strcpy(szPath, szSearchPath);
	if (szPath[strlen(szPath)-1] != '/') strcat(szPath, "/");
	char szFullPath[255];
	char *szDlerror = NULL;
	if (dir) 
	{
		logprintf("");
		logprintf("Server Plugins");
		logprintf("--------------");

		// Load the plugins
		while (dir) 
		{
			strcpy(szFullPath, szPath);
			if ((file = readdir(dir)) != NULL) 
			{
				szFilename = (char*)file->d_name;
				iStrLen = strlen(szFilename);
				if (iStrLen > 3) 
				{
					szExt = szFilename+iStrLen-3;
					if (strcmp(".so", szExt) == 0) 
					{
						logprintf(" Loading plugin: %s", szFilename);
						strcat(szFullPath, szFilename);
						bool bSuccess = LoadSinglePlugin(szFullPath);
						if (!bSuccess) 
						{
							szDlerror = PLUGIN_GETERROR();
							if (szDlerror) 
								logprintf("  Failed (%s)", szDlerror);
							else 
								logprintf("  Failed.");
						}
					}
				}
			} 
			else 
			{
				closedir(dir);
				dir = NULL;
			}
		}
		logprintf("");
	}
#else
	char szPath[MAX_PATH];
	char szSearch[MAX_PATH];

	strcpy(szPath, szSearchPath);

	char cLastChar = szPath[strlen(szPath)-1];
	if (cLastChar != '\\' && cLastChar != '/')
		strcat(szPath, "\\");

	DWORD szPathLen = strlen(szPath);

	strcpy(szSearch, szPath);
	strcat(szSearch, "*.dll");

	WIN32_FIND_DATA fdFindData;
	HANDLE hFindFile = FindFirstFile(szSearch, &fdFindData);
	if (hFindFile != INVALID_HANDLE_VALUE) 
	{
		logprintf("");
		logprintf("Server Plugins");
		logprintf("--------------");
		
		// Load the plugins
		while(true) {
			logprintf(" Loading plugin: %s", fdFindData.cFileName);
			strcpy(szPath+szPathLen, fdFindData.cFileName);
			BOOL bSuccess = LoadSinglePlugin(szPath);
			if (!bSuccess)
				logprintf("  Failed.");
			if(!FindNextFile(hFindFile, &fdFindData))
				break;
		}
		logprintf("");
	}
	FindClose(hFindFile);
#endif
}

//---------------------------------------
	
DWORD CPlugins::GetPluginCount()
{
	return (DWORD)m_Plugins.size();
}

//---------------------------------------

ServerPlugin_s* CPlugins::GetPlugin(DWORD index)
{
	return m_Plugins[index];
}

//---------------------------------------

void CPlugins::DoProcessTick()
{

	ServerPluginVector::iterator itor;

	for(itor=m_Plugins.begin(); itor!=m_Plugins.end(); itor++) {
		ServerPlugin_s* pSPlugin = *itor;
		if ((pSPlugin->dwSupportFlags & SUPPORTS_PROCESS_TICK) != 0)
			if (pSPlugin->ProcessTick != NULL)
				pSPlugin->ProcessTick();
	}

}

//---------------------------------------

void CPlugins::DoAmxLoad(AMX *amx) 
{

	ServerPluginVector::iterator itor;

	for(itor=m_Plugins.begin(); itor!=m_Plugins.end(); itor++) {
		ServerPlugin_s* pSPlugin = *itor;
		if ((pSPlugin->dwSupportFlags & SUPPORTS_AMX_NATIVES) != 0)
			if (pSPlugin->AmxLoad != NULL)
				pSPlugin->AmxLoad(amx);
	}

}

//---------------------------------------

void CPlugins::DoAmxUnload(AMX *amx)
{

	ServerPluginVector::iterator itor;

	for(itor=m_Plugins.begin(); itor!=m_Plugins.end(); itor++) 
	{
		ServerPlugin_s* pSPlugin = *itor;
		if ((pSPlugin->dwSupportFlags & SUPPORTS_AMX_NATIVES) != 0)
			if (pSPlugin->AmxUnload != NULL)
				pSPlugin->AmxUnload(amx);
	}

}

//---------------------------------------

void CHelpDialog::Draw()
{
	RECT rect;
	rect.top		= 10;
	rect.right		= pGame->GetScreenWidth() - 150;
	rect.left		= 10;
	rect.bottom		= rect.top + 16;

	pDefaultFont->RenderText("--- SA:MP Key Binds ---",rect,0xFFFFFFFF); rect.top += 16; rect.bottom += 16;
	pDefaultFont->RenderText("F1: Display this help dialog",rect,0xFFFFFFFF); rect.top += 16; rect.bottom += 16;
	pDefaultFont->RenderText("TAB: Display the scoreboard",rect,0xFFFFFFFF); rect.top += 16; rect.bottom += 16;
	pDefaultFont->RenderText("F4: Allows you to change class next time you respawn",rect,0xFFFFFFFF); rect.top += 16; rect.bottom += 16;
	pDefaultFont->RenderText("F5: Show bandwidth statistics",rect,0xFFFFFFFF); rect.top += 16; rect.bottom += 16;
	pDefaultFont->RenderText("F7: Toggle the chat box",rect,0xFFFFFFFF); rect.top += 16; rect.bottom += 16;
	pDefaultFont->RenderText("F8: Take a screenshot",rect,0xFFFFFFFF); rect.top += 16; rect.bottom += 16;
	pDefaultFont->RenderText("F9: Toggle the deathwindow",rect,0xFFFFFFFF); rect.top += 16; rect.bottom += 16;
	pDefaultFont->RenderText("F11: Change font",rect,0xFFFFFFFF); rect.top += 16; rect.bottom += 16;
	pDefaultFont->RenderText("T/F6: Allows you to enter a chat message",rect,0xFFFFFFFF); rect.top += 16; rect.bottom += 16;
}



#define DEVELOPER "kye"
#define SAMP_VERSION "0.3.7"
#define BUILD_INFO "SA-MP-" SAMP_VERSION "-" DEVELOPER "-" __DATE__ "-" __TIME__





void ServerCommand(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;
	int iStrLen=0;
	unsigned char* szCommand=NULL;

	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);

	if(pNetGame->GetGameState() != GAMESTATE_RUNNING) return;

	BYTE bytePlayerID = pRak->GetIndexFromPlayerID(sender);
		
	bsData.Read(iStrLen);

	if(iStrLen < 1) return;
	if(iStrLen > MAX_CMD_INPUT) return;

	szCommand = (unsigned char*)calloc(iStrLen+1,1);
	bsData.Read((char*)szCommand, iStrLen);
	szCommand[iStrLen] = '\0';

	ReplaceBadChars((char *)szCommand);

	if (!pNetGame->GetFilterScripts()->OnPlayerCommandText(bytePlayerID, szCommand))
	{
		if (pNetGame->GetGameMode())
		{
			if (!pNetGame->GetGameMode()->OnPlayerCommandText(bytePlayerID, szCommand))
			{
				pNetGame->SendClientMessage(sender, 0xFFFFFFFF, "SERVER: Unknown command.");
			}
		}
	}

	free(szCommand);
}

void UpdateScoresPingsIPs(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	RakNet::BitStream bsData(Data,(iBitLength/8)+1,false);

	RakNet::BitStream bsParams;
	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
	BYTE bytePlayerId = pRak->GetIndexFromPlayerID(sender);

	if(!pPlayerPool->GetSlotState(bytePlayerId)) return;

	for (BYTE i=0; i<MAX_PLAYERS; i++)
	{
		if (pPlayerPool->GetSlotState(i))
		{
			bsParams.Write(i);
			bsParams.Write((DWORD)pPlayerPool->GetPlayerScore(i));
			bsParams.Write((DWORD)pRak->GetLastPing(pRak->GetPlayerIDFromIndex(i)));
		}
	}

	pRak->RPC(RPC_UpdateScoresPingsIPs, &bsParams, HIGH_PRIORITY, RELIABLE, 0, sender, false, false);
}

float CObject::DistanceRemaining()
{
	float	fSX,fSY,fSZ;
	fSX = (m_matWorld.pos.X - m_matTarget.pos.X) * (m_matWorld.pos.X - m_matTarget.pos.X);
	fSY = (m_matWorld.pos.Y - m_matTarget.pos.Y) * (m_matWorld.pos.Y - m_matTarget.pos.Y);
	fSZ = (m_matWorld.pos.Z - m_matTarget.pos.Z) * (m_matWorld.pos.Z - m_matTarget.pos.Z);
	return (float)sqrt(fSX + fSY + fSZ);
}


void CPlayer::StoreAimSyncData(AIM_SYNC_DATA *paimSync)
{
	memcpy(&m_aimSync,paimSync,sizeof(AIM_SYNC_DATA));
	m_bHasAimUpdates = TRUE;
}

void CPlayer::HandleDeath(BYTE byteReason, BYTE byteWhoWasResponsible)
{
	RakNet::BitStream bsPlayerDeath;
	PlayerID playerid = pNetGame->GetRakServer()->GetPlayerIDFromIndex(m_bytePlayerID);

	SetState(PLAYER_STATE_WASTED);
	
	pNetGame->GetFilterScripts()->OnPlayerDeath((cell)m_bytePlayerID, (cell)byteWhoWasResponsible, (cell)byteReason);
	CGameMode *pGameMode = pNetGame->GetGameMode();
	if(pGameMode) pGameMode->OnPlayerDeath((cell)m_bytePlayerID, (cell)byteWhoWasResponsible, (cell)byteReason);

	bsPlayerDeath.Write(m_bytePlayerID);

	pNetGame->GetRakServer()->RPC(RPC_Death,&bsPlayerDeath,
		HIGH_PRIORITY,RELIABLE,0,playerid,true,false);


void CPlayer::SetPlayerColor(DWORD dwColor)
{
	RakNet::BitStream bsColor;
	
	m_dwColor = dwColor;

	bsColor.Write(m_bytePlayerID);
	bsColor.Write(dwColor);

	pNetGame->GetRakServer()->RPC(RPC_ScrSetPlayerColor,&bsColor,HIGH_PRIORITY,RELIABLE,
		0,UNASSIGNED_PLAYER_ID,true,false);
}

/* SetObjectVirtualWorld */

..........

// Slightly encrypted strings for the APIs...
char szGetWindowThreadProcessId[25]	= { 0x47, 0x34, 0xD6, 0xA4, 0x2D, 0xFB, 0x82, 0x58, 0xFF, 0x8D,
										0x42, 0x09, 0xA9, 0x7C, 0x0A, 0xEF, 0x62, 0x0E, 0xD1, 0x66,
										0x27, 0xD6, 0xBF, 0x23, 0x00 };
char szFindWindowA[12]				= { 0x46, 0x38, 0xCC, 0x97, 0x13, 0xFC, 0x88, 0x53, 0xE7, 0xAE,
										0x6B, 0x00 };
char szSetWindowsHookExA[18]		= { 0x53, 0x34, 0xD6, 0xA4, 0x2D, 0xFB, 0x82, 0x58, 0xFF, 0xAA,
										0x62, 0x14, 0xA3, 0x76, 0x2B, 0xC7, 0x51, 0x00 };
char szUnhookWindowsHookEx[20]		= { 0x55, 0x3F, 0xCA, 0x9C, 0x2B, 0xFE, 0xB1, 0x5E, 0xE6, 0xBD,
										0x45, 0x0C, 0xBF, 0x55, 0x01, 0xD0, 0x7B, 0x24, 0xCA, 0x00 };
char szCallNextHookEx[15]			= { 0x43, 0x30, 0xCE, 0x9F, 0x0A, 0xF0, 0x9E, 0x43, 0xC0, 0xB6,
										0x45, 0x10, 0x89, 0x65, 0x00 };
char szOpenProcess[12]				= { 0x4F, 0x21, 0xC7, 0x9D, 0x14, 0xE7, 0x89, 0x54, 0xED, 0xAA,
										0x59, 0x00 };
char szCloseHandle[12]				= { 0x43, 0x3D, 0xCD, 0x80, 0x21, 0xDD, 0x87, 0x59, 0xEC, 0xB5,
										0x4F, 0x00 };
char szWriteProcessMemory[19]		= { 0x57, 0x23, 0xCB, 0x87, 0x21, 0xC5, 0x94, 0x58, 0xEB, 0xBC,
										0x59, 0x08, 0x81, 0x78, 0x03, 0xD0, 0x62, 0x18, 0x00 };
char szCreateRemoteThread[19]		= { 0x43, 0x23, 0xC7, 0x92, 0x30, 0xF0, 0xB4, 0x52, 0xE5, 0xB6,
										0x5E, 0x1E, 0x98, 0x75, 0x1C, 0xDA, 0x71, 0x05, 0x00 };
char szGrandTheftAutoSA[29]			= { 0x47, 0x23, 0xC3, 0x9D, 0x20, 0xB5, 0x92, 0x5F, 0xED, 0xBF,
										0x5E, 0x5B, 0xAD, 0x68, 0x1A, 0xD0, 0x30, 0x32, 0xD3, 0x6D,
										0x74, 0xE4, 0x98, 0x23, 0xEA, 0x8C, 0x5B, 0xF8, 0x00 };
char szGTASanAndreas[17]			= { 0x47, 0x05, 0xE3, 0xC9, 0x64, 0xC6, 0x87, 0x59, 0xA8, 0x98,
										0x44, 0x1F, 0xBE, 0x78, 0x0F, 0xCC, 0x00 };






//

void CDeathWindow::CreateFonts()
{
	if(!m_pD3DDevice) return;
	
	RECT rectLongestNick = {0,0,0,0};
	int	iFontSize;

	if(m_pD3DFont) SAFE_DELETE(m_pD3DFont);
	if(m_pWeaponFont) SAFE_DELETE(m_pWeaponFont);
	if(m_pSprite) SAFE_DELETE(m_pSprite);

	// Create a sprite to use when drawing text
	D3DXCreateSprite(m_pD3DDevice,&m_pSprite);
		
	if(pGame->GetScreenWidth() <= 1024) {
		iFontSize = 12;
	} else {
		iFontSize = 14;
	}

	D3DXCreateFont(m_pD3DDevice, iFontSize, 0, FW_BOLD, 1, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, "Arial", &m_pD3DFont);

	// Store the rect for right aligned name (DT_RIGHT fucks the text)
	if(m_pD3DFont) m_pD3DFont->DrawText(0,"LONGESTNICKNICKK",-1,&rectLongestNick,DT_CALCRECT|DT_LEFT,0xFF000000);
		
	m_iLongestNickLength = rectLongestNick.right - rectLongestNick.left;
	
	D3DXCreateFont(m_pD3DDevice, 20, 0, FW_NORMAL, 1, FALSE,
		SYMBOL_CHARSET, OUT_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, "GTAWEAPON3", &m_pWeaponFont);
}

void CSvrNetStats::Draw()
{
	return;

	float fDown,fUp;

	if((GetTickCount() - m_dwLastUpdateTick) > 1000) {
		m_dwLastUpdateTick = GetTickCount();

		m_dwBPSDownload = ((UINT)(RakServerStats.bitsReceived / 8)) - m_dwLastTotalBytesRecv;
		m_dwLastTotalBytesRecv = (UINT)(RakServerStats.bitsReceived / 8);
		m_dwBPSUpload = ((UINT)(RakServerStats.totalBitsSent / 8)) - m_dwLastTotalBytesSent;
		m_dwLastTotalBytesSent = (UINT)(RakServerStats.totalBitsSent / 8);

		RakNet::BitStream bsParams;
		pNetGame->GetRakClient()->RPC(RPC_SvrStats,&bsParams,HIGH_PRIORITY,UNRELIABLE,0,false);
	}

	if(m_dwBPSDownload != 0) {
		fDown = (float)m_dwBPSDownload / 1024;
	} else {
		fDown = 0.0f;
	}

	if(m_dwBPSUpload != 0) {
		fUp = (float)m_dwBPSUpload / 1024;
	} else {
		fUp = 0.0f;
	}

	sprintf(szSvrDispBuf,"Download Rate: %.2f kbps\nUpload Rate: %.2f kbps\n",fDown,fUp);
	StatisticsToString(&RakServerStats,szSvrStatBuf,1);
	strcat(szSvrDispBuf,szSvrStatBuf);

	RECT rect;
	rect.top		= 10;
	rect.right		= pGame->GetScreenWidth() - 150;
	rect.left		= 10;
	rect.bottom		= rect.top + 16;

	PCHAR pBuf = szSvrDispBuf;
	
	int x=0;

	pDefaultFont->RenderText("Server Net Stats (Admin only)",rect,0xFFAA0000);
	rect.top += 16;
	rect.bottom += 16;

	while(*pBuf) {
		szSvrDrawLine[x] = *pBuf;
		if(szSvrDrawLine[x] == '\n') {
			szSvrDrawLine[x] = '\0';
			pDefaultFont->RenderText(szSvrDrawLine,rect,0xFFFFFFFF);
			rect.top += 16;
			rect.bottom += 16;
			x=0;
		} else {
			x++;
		}
		pBuf++;
	}
}

OnPlayerCollide :

// forward OnPlayerCollide(playerid, collideid, objectid, vehicleid);
int CGameMode::OnPlayerCollide(cell playerid, cell collideid, cell objectid, cell vehicleid)
{
	CHECK_INIT();

	int idx;
	cell ret = 0;

	if (!amx_FindPublic(&m_amx, "OnPlayerCollide", &idx))
	{
		amx_Push(&m_amx, vehicleid);
		amx_Push(&m_amx, objectid);
		amx_Push(&m_amx, collideid);
		amx_Push(&m_amx, playerid);
		amx_Exec(&m_amx, &ret, idx);
	}
	return (int)ret;
}

// ========================================

Nitro : 

// forward OnPlayerActiveNos(playerid, vehicleid) // Nitro
int CGameMode::OnPlayerActiveNos(cell playerid, cell vehicleid)
{
	CHECK_INIT();

	int idx;
	cell ret = 0;

	if (!amx_FindPublic(&m_amx, "OnPlayerActiveNos", &idx))
	{
		amx_Push(&m_amx, vehicleid);
		amx_Push(&m_amx, playerid);
		amx_Exec(&m_amx, &ret, idx);
	}
	return (int)ret;
}


// ===========================================

Create3dProgressBar : 

// native Create3DProgressBar(color, Float, ..........);
static cell AMX_NATIVE_CALL n_Create3DProgressBar(AMX *amx, cell *params)
{
	int iBarId = params[1];
	if(!pNetGame->GetPlayerPool()) return 0;
	if(!pNetGame->GetPlayerPool()->GetSlotState(params[2])) return 0;

	BAR Bar;
    Bar.iColor = params[3];
	Bar.fX = amx_ctof(params[4]);
	Bar.fY = amx_ctof(params[5]);
	Bar.fZ = amx_ctof(params[6]);

	RakNet::BitStream bsBar;
	bsBar.Write(iBarId);
	bsBar.Write((PCHAR)&Bar,sizeof(BAR));
	pNetGame->GetRakServer()-> // [ ... ]
	// [ ... ]
		pNetGame->GetRakServer()->GetPlayerIDFromIndex(params[2]), false, false);
	// print .
	return 1;
}



//

Archive fix : 

for(DWORD i=0; i<dwFileSize; i+=BUFFER_SIZE)
{
	DWORD dwReadBytes;
	if ((i + BUFFER_SIZE) >= dwFileSize) {
		dwReadBytes = dwFileSize % BUFFER_SIZE;
		} else {
			dwReadBytes = BUFFER_SIZE;
		}
		dwReadBytes = (DWORD)fread(pbBuffer, 1, dwReadBytes, fiData);
		if (memcmp(pbBuffer, pbFileData+i, dwReadBytes) != 0) {
			throw "  Failed to verify file.\n";
		}				
}

//======================================

	if (byteRconUser != INVALID_ID)
	{
		pNetGame->SendClientMessage(pNetGame->GetRakServer()->GetPlayerIDFromIndex(byteRconUser), 0xFFFFFFFF, buffer);
	} else if (bRconSocketReply) {
		RconSocketReply(buffer);
	}

//SetPlayerName FIX:

//native SetPlayerName(playerid, name[])
static cell AMX_NATIVE_CALL n_SetPlayerName(AMX *amx, cell *params)
{
	CHECK_PARAMS(2);

	if (pNetGame->GetPlayerPool()->GetSlotState(BYTE(params[1])))
	{
		char *szNewNick;
		char szOldNick[MAX_PLAYER_NAME+1];
		amx_StrParam(amx, params[2], szNewNick);

		if(ContainsInvalidNickChars(szNewNick)) return -1;

		BYTE bytePlayerID = (BYTE)params[1];
		BYTE byteNickLen = strlen(szNewNick);
		BYTE byteSuccess;

		if(byteNickLen > MAX_PLAYER_NAME) return -1;

		strncpy(szOldNick,pNetGame->GetPlayerPool()->GetPlayerName(bytePlayerID),MAX_PLAYER_NAME);
		
		if (byteNickLen == 0 || pNetGame->GetPlayerPool()->IsNickInUse(szNewNick)) byteSuccess = 0;
		else byteSuccess = 1;

		RakNet::BitStream bsData;
		bsData.Write(bytePlayerID); // player id
		bsData.Write(byteNickLen); // nick length
		bsData.Write(szNewNick, byteNickLen); // name
		bsData.Write(byteSuccess); // if the nickname was rejected

		if (byteSuccess != 0)
		{
			pNetGame->GetPlayerPool()->SetPlayerName(bytePlayerID, szNewNick);
			logprintf("[nick] %s nick changed to %s", szOldNick, szNewNick);
			pNetGame->GetRakServer()->RPC(RPC_ScrSetPlayerName , &bsData, HIGH_PRIORITY, RELIABLE, 0, UNASSIGNED_PLAYER_ID, true, false);

		}

		return byteSuccess;
	}
	else
	{
		return 0;
	}
}

//ClientChecks
        {0x8C,0x66,0x8C,0x27,0xC5,0x8C,0x8D,0x8D,0};
        //0x03,0x02,0x05 / Cleo, RakSamp, S0beit 
 	/*
		SendClientCheck(playerid, 0x05, ..., ..., ...); // etc .
	*/
//

// ==================

if(memcmp(&pPed->WeaponSlots[0],byteWeaponData,WEAPON_DATA_SIZE) != 0) {
        //
	
	//
        iTimesDataModified++;
}

// ==================
 
// FIX Memory BUG :
 
        if (bAsDWords)
        {      
                //
                for(int i=0; i<(int)dwCount; i += 85) // 98
                //
                {
			// DWORD 6 - 11
                }
		//
        }
 
//===================================
 
//vector :
 
        //vector compression fix :
       
        int X = (int)(vec->X * 8358607.4f);
        int Y = (int)(vec->Y * 8358607.4f);
        int Z = (int)(vec->Z * 8358607.4f);
 
        memcpy(c1->data+0, &X, 3);
        memcpy(c1->data+3, &Y, 3);
        memcpy(c1->data+6, &Z, 3);
 
// =========================
 
//AddStaticVehicleEx :
 
static cell AMX_NATIVE_CALL n_AddStaticVehicleEx(AMX *amx, cell *params)
{
        CHECK_PARAMS(8);
 
        VECTOR vecPos;
        vecPos.X = amx_ctof(params[2]);
        vecPos.Y = amx_ctof(params[3]);
        vecPos.Z = amx_ctof(params[4]);
 
        /*
                GetVehiclePool
        */
        VEHICLEID ret = pNetGame->GetVehiclePool()->New((int)params[1], &vecPos, amx_ctof(params[5]),
                (int)params[6], (int)params[7], ((int)params[8]) * 1100); // params[8] * 1100
 
        return ret;
}
 
// =====================================
 
// ApplyAnimation client-crash :
 
static cell AMX_NATIVE_CALL n_ApplyAnimation(AMX *amx, cell *params)
{
        CHECK_PARAMS(9);
        RakNet::BitStream bsSend;
 
        char *szAnimLib;
        char *szAnimName;
        BYTE byteAnimLibLen;
        BYTE byteAnimNameLen;
        float fS;
        bool opt1,opt2,opt3,opt4;
        int opt5;
       
        CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
        if(!pPlayerPool || !pPlayerPool->GetSlotState(params[1])) return 1;
 
        amx_StrParam(amx, params[2], szAnimLib);
        amx_StrParam(amx, params[3], szAnimName);
 
        byteAnimLibLen = strlen(szAnimLib);
        byteAnimNameLen = strlen(szAnimName);
 
        fS = amx_ctof(params[4]);
        opt1 = (bool)params[5];
        opt2 = (bool)params[6];
        opt3 = (bool)params[7];
        opt4 = (bool)params[8];
        opt5 = (int)params[9];
 
        bsSend.Write((BYTE)params[1]);
        bsSend.Write(byteAnimLibLen);
        bsSend.Write(szAnimLib,byteAnimLibLen);
        bsSend.Write(byteAnimNameLen);
        bsSend.Write(szAnimName,byteAnimNameLen);
        bsSend.Write(fS);
        bsSend.Write(opt1);
        bsSend.Write(opt2);
        bsSend.Write(opt3);
        bsSend.Write(opt4);
        bsSend.Write(opt5);
 
        /*
                >
        */
        pNetGame->BroadcastDistanceRPC(RPC_ScrApplyAnimation,&bsSend,UNRELIABLE,(BYTE)params[1],200.1f);
       
        /*
                Why return 0 and true ? ?
        */
                //return 0;
        return true;
}
 
//===============================
 
//
 
                while (!bFound && i < byteCount)
                {
                        pItem = (BINARY_TREE*)(pbData + (i * sizeof (BINARY_TREE)));
 
                        // ... HERE ! ...
 
                        else if (pItem->dwFileHash < dwFileHash)
                        {
                                i = pItem->byteRight;
                        }
                        else {
                                i = pItem->byteLeft;
                        }
                }
 
// CarJack  : 
	
	//  If a customer have CLEO modifications, OnPlayerStateChange will cause crash.

	if(byteNewState == PLAYER_STATE_DRIVER && byteOldState == PLAYER_STATE_ONFOOT)
	{
		/*
		[ code ] 
		*/

		// FIX :
		if(pLocalPlayerPed && pLocalPlayerPed->IsInVehicle() && !pLocalPlayerPed->IsAPassenger())
		{
			LocalVehicle = pNetGame->GetVehiclePool()->FindIDFromGtaPtr(pLocalPlayerPed->GetGtaVehicle());
			if(LocalVehicle == m_VehicleID) {
				//  [    ....     ]
				pLocalPlayerPed->RemoveFromVehicleAndPutAt(mat.pos.X,mat.pos.Y,mat.pos.Z + 1.0f);	
				//carjacked
			}
		}
	}


// ====================================

	if(m_pPlayerPed && pVehicle && !m_pPlayerPed->IsInVehicle()) {

		if(bPassenger) {
			SetState(PLAYER_STATE_ENTER_VEHICLE_PASSENGER);
		} else {
			SetState(PLAYER_STATE_ENTER_VEHICLE_DRIVER);
		}
		
		// Kye :
 
		m_pPlayerPed->SetKeys(0,0,0); // Keys set to 0,0,0

		// [ ....]

//======================== La Pirula Project - BIKE CRASH FIX ==============================


if (pVehicleSync.fTrainSpeed > 1000.0 || pVehicleSync.fTrainSpeed < 0.0)
{
	return 0;
}


// ============= TEXT BUFFER CRASH ? =============

while(*szText)
{
	byte c = *szText++;

	const float *pfChar = m_pFont->GetCharCoords(c);

	float tx1 = pfChar[0];
	float tx2 = pfChar[2];

	// ... HERE ...

}
// return

// ==========================
// WTF ?

float DegToRad(float fDegrees)
{
	if (fDegrees > 360.0f || fDegrees < 0.0f) return 0.0f;
	if (fDegrees > 180.0f) {
		//return (float)(-(PI - (((fDegrees - 360.0f) * PI) / 180.0f))); // seriously ?
		return (float)(-(PI - (((fDegrees - 180.0f) * PI) / 180.0f)));
	} else {
		//return float()((fDegrees / 400 * PI / 180.0f); //WTF IS THIS KYE ???

		//Look :
		return (float)((fDegrees * PI) / 180.0f); 
	}
}

// ===================

	if(m_dwBPSUpload != 0) {
		fUp = (float)m_dwBPSUpload / 1024;
	} else {
		// [ omg, please look here ... ]
	}