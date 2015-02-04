// This is from RakNet sources.
BYTE GetPacketID(Packet *p) 
{
	if (p==0) return 255;

	if ((unsigned char)p->data[0] == ID_TIMESTAMP) {
		assert(p->length > sizeof(unsigned char) + sizeof(unsigned long));
		return (unsigned char) p->data[sizeof(unsigned char) + sizeof(unsigned long)];
	}
	else return (unsigned char) p->data[0];
}

//==========================================================
netgame


CNetGame::CNetGame()
{
	fRestartWaitTime = 0.0f;
	m_bAdminTeleport = false;
	m_bAllowWeapons = FALSE;
	m_byteWorldTime = 12;
	m_byteWeather	= 10;
	m_bStuntBonus   = true;
	m_fGravity		= 0.008f;
	m_iDeathDropMoney = 0;
	m_bZoneNames = FALSE;
	
	m_longSynchedWeapons = DEFAULT_WEAPONS;
	// Change number here and in ShutdownForGameModeRestart for default weapon sets

	// Init member variables
	m_pPlayerPool = NULL;
	m_pVehiclePool = NULL;
	m_pGameMode = NULL;
	m_pPickupPool = NULL;
	m_pObjectPool = NULL;
	m_pMenuPool = NULL;
	m_pTextPool = NULL;
	m_pGangZonePool = NULL;
	m_bLanMode = FALSE;
	m_byteMod = 0x01;
	m_bACEnabled = pConsole->GetBoolVariable("anticheat");

	m_bLimitGlobalChatRadius = FALSE;
	m_fGlobalChatRadius = 10000.0f;
	m_fNameTagDrawDistance = 70.0f;
	m_bDisableEnterExits = false;

	m_iCurrentGameModeIndex = 0;
	m_iCurrentGameModeRepeat = 0;
	m_bFirstGameModeLoaded = FALSE;
	m_pScriptTimers = new CScriptTimers;
	
	#ifndef WIN32
		m_dElapsedTime = 0.0;
	#endif

	if(pConsole->GetIntVariable("maxplayers") > MAX_PLAYERS) {
		pConsole->SetIntVariable("maxplayers", MAX_PLAYERS);
	}

	CHAR *szBindAddress = pConsole->GetStringVariable("bind");
	if (szBindAddress && szBindAddress[0] == 0)
		szBindAddress = NULL;

	DWORD dwPort = pConsole->GetIntVariable("port");
	DWORD dwMaxPlayers = pConsole->GetIntVariable("maxplayers");
	BOOL bLanMode = pConsole->GetBoolVariable("lanmode");
	BOOL bMyriad = pConsole->GetBoolVariable("myriad");

	// Setup RakNet
	m_pRak = RakNetworkFactory::GetRakServerInterface();
	//m_pRak->InitializeSecurity(0, 0);
	//m_pRak->SetTrackFrequencyTable(true);

	if (!m_pRak->Start(dwMaxPlayers, 0, 10, dwPort, szBindAddress))
	{
		if (szBindAddress)
			logprintf("Unable to start server on %s:%d. Port in use?", 
							szBindAddress, dwPort);
		else
			logprintf("Unable to start server on port: %d. Port in use?", dwPort);
		return;
	}
	
	LoadBanList();

	if(!SetNextScriptFile(NULL)) {
		logprintf("I couldn't load any gamemode scripts. Please verify your server.cfg");
		logprintf("It needs a gamemode0 line at the very least.");
		fcloseall();
		exit(1);
	}
		
	m_pRak->StartOccasionalPing();
	//m_pRak->SetMTUSize(1492);

	char* szPass = pConsole->GetStringVariable("password");
	if ((szPass) && (szPass[0] != 0)) { 
		m_pRak->SetPassword(szPass);
	}

	// Register our RPC handlers
	RegisterRPCs(m_pRak);

	//if (IsACEnabled())
		//CAntiCheat::Initialize(this);

	char szTime[256];
	sprintf(szTime, "%02d:%02d", m_byteWorldTime, 0);
	pConsole->AddStringVariable("worldtime", CON_VARFLAG_RULE, szTime);

	
	// Define LAN mode
	if(bLanMode) {
		m_bLanMode = TRUE;
	}

	char szScriptFiles[512];
	int len;

	#ifdef WIN32
		GetCurrentDirectory(sizeof(szScriptFiles), szScriptFiles);
		len = strlen(szScriptFiles);
		if (szScriptFiles[len-1] != '\\')
		{
			szScriptFiles[len] = '\\';
			szScriptFiles[len+1] = '\0';
		}
		strcat(szScriptFiles, "scriptfiles\\");
		SetEnvironmentVariable("AMXFILE", szScriptFiles);
	#else
		getcwd(szScriptFiles, sizeof(szScriptFiles));
		len = strlen(szScriptFiles);
		if (szScriptFiles[len-1] != '/')
		{
			szScriptFiles[len] = '/';
			szScriptFiles[len+1] = '\0';
		}
		strcat(szScriptFiles, "scriptfiles/");
		setenv("AMXFILE", szScriptFiles, 1);
	#endif

	// Filterscripts are now entirely controlled from here, were all done from Gamemode before
	m_pFilterScripts = new CFilterScripts();
	
	if (szBindAddress) {
		printf( " Started Server on %s:%d, with maxplayers: %d lanmode is %s.\n",
			szBindAddress, dwPort, dwMaxPlayers, bLanMode?"ON":"OFF" );
	} else {
		printf( " Started Server on port: %d, with maxplayers: %d lanmode is %s.\n",
			dwPort, dwMaxPlayers, bLanMode?"ON":"OFF" );
	}

	m_iGameState = GAMESTATE_STOPPED;
}

		case ID_PLAYER_SYNC:
			Packet_PlayerSync(p);
			break;
		case ID_VEHICLE_SYNC:
			Packet_VehicleSync(p);
			break;
		case ID_PASSENGER_SYNC:
			Packet_PassengerSync(p);
			break;
		case ID_SPECTATOR_SYNC:
			Packet_SpectatorSync(p);
			break;
		case ID_AIM_SYNC:
			Packet_AimSync(p);
			break;
		case ID_RCON_COMMAND:
			Packet_InGameRcon(p);
			break;
		case ID_STATS_UPDATE:
			Packet_StatsUpdate(p);
			break;
		case ID_WEAPONS_UPDATE:
			Packet_WeaponsUpdate(p);
			break;
		case ID_TRAILER_SYNC:
			Packet_TrailerSync(p);
			break;

int CNetGame::GetBroadcastSendRateFromPlayerDistance(float fDistance)
{
	if(fDistance < 75.0f)	return 0;
	if(fDistance < 150.0f)	return 1;
	if(fDistance < 220.0f)	return 2;
	if(fDistance < 250.0f)	return 15;
	if(fDistance < 300.0f)	return 20;
	if(fDistance < 400.0f)	return 25;
	if(fDistance < 500.0f)	return 30;
	if(fDistance < 750.0f)	return 35;
	if(fDistance < 1000.0f) return 40;
	if(fDistance < 1500.0f) return 45;
	if(fDistance < 2000.0f) return 50;

	return 75;
}
//--------------------------------------------------------

void CNetGame::AdjustAimSync(RakNet::BitStream *bitStream, BYTE byteTargetPlayerID, RakNet::BitStream *adjbitStream)
{
	BYTE bytePlayerID, bytePacketID;
	int iPlayerPing, iTargetPing;
	AIM_SYNC_DATA aimSync;
	ONFOOT_SYNC_DATA *ofSync;

	bitStream->Read(bytePacketID);
	bitStream->Read(bytePlayerID);
	bitStream->Read((PCHAR)&aimSync, sizeof(AIM_SYNC_DATA));
	bitStream->ResetReadPointer();

	iPlayerPing = m_pRak->GetLastPing(m_pRak->GetPlayerIDFromIndex(bytePlayerID));
	iTargetPing = m_pRak->GetLastPing(m_pRak->GetPlayerIDFromIndex(byteTargetPlayerID));
	ofSync = GetPlayerPool()->GetAt(byteTargetPlayerID)->GetOnFootSyncData();

	if (ofSync) {
		aimSync.vecAimPos.X += ofSync->vecMoveSpeed.X * (iPlayerPing + iTargetPing) / 10;
		aimSync.vecAimPos.Y += ofSync->vecMoveSpeed.Y * (iPlayerPing + iTargetPing) / 10;
		aimSync.vecAimPos.Z += ofSync->vecMoveSpeed.Z * (iPlayerPing + iTargetPing) / 10;
	}

	adjbitStream->Write(bytePacketID);
	adjbitStream->Write(bytePlayerID);
	adjbitStream->Write((PCHAR)&aimSync, sizeof(AIM_SYNC_DATA));
}

//----------------------------------------------------
// PACKET HANDLERS
//----------------------------------------------------

void CNetGame::Packet_PlayerSync(Packet *p)
{
	CPlayer * pPlayer = GetPlayerPool()->GetAt((BYTE)p->playerIndex);
	RakNet::BitStream bsPlayerSync((PCHAR)p->data, p->length, false);

	if(GetGameState() != GAMESTATE_RUNNING) return;

	BYTE				bytePacketID=0;
	ONFOOT_SYNC_DATA	ofSync;
	
	bsPlayerSync.Read(bytePacketID);
	bsPlayerSync.Read((PCHAR)&ofSync,sizeof(ONFOOT_SYNC_DATA));
	
	if(pPlayer)	{
		pPlayer->StoreOnFootFullSyncData(&ofSync);
	}
}

//----------------------------------------------------

void CNetGame::Packet_AimSync(Packet *p)
{
	CPlayer * pPlayer = GetPlayerPool()->GetAt((BYTE)p->playerIndex);
	RakNet::BitStream bsPlayerSync((PCHAR)p->data, p->length, false);

	if(GetGameState() != GAMESTATE_RUNNING) return;

	BYTE			bytePacketID=0;
	AIM_SYNC_DATA	aimSync;
	
	bsPlayerSync.Read(bytePacketID);
	bsPlayerSync.Read((PCHAR)&aimSync,sizeof(AIM_SYNC_DATA));
		
	if(pPlayer)	{
		pPlayer->StoreAimSyncData(&aimSync);
	}
}

//----------------------------------------------------

void CNetGame::Packet_VehicleSync(Packet *p)
{
	CPlayer * pPlayer = GetPlayerPool()->GetAt((BYTE)p->playerIndex);
	RakNet::BitStream bsVehicleSync((PCHAR)p->data, p->length, false);

	if(GetGameState() != GAMESTATE_RUNNING) return;

	BYTE		bytePacketID=0;
	INCAR_SYNC_DATA icSync;
	
	bsVehicleSync.Read(bytePacketID);
	bsVehicleSync.Read((PCHAR)&icSync,sizeof(INCAR_SYNC_DATA));

	if(pPlayer)	{
		if(icSync.VehicleID == 0xFFFF) {
			// Syncing a manually added/bad vehicle
			KickPlayer((BYTE)p->playerIndex);
			return;
		}
	    pPlayer->StoreInCarFullSyncData(&icSync);
	}
}

//----------------------------------------------------

void CNetGame::Packet_PassengerSync(Packet *p)
{
	CPlayer * pPlayer = GetPlayerPool()->GetAt((BYTE)p->playerIndex);
	RakNet::BitStream bsPassengerSync((PCHAR)p->data, p->length, false);

	if(GetGameState() != GAMESTATE_RUNNING) return;

	BYTE bytePacketID=0;
	PASSENGER_SYNC_DATA psSync;
	
	bsPassengerSync.Read(bytePacketID);
	bsPassengerSync.Read((PCHAR)&psSync,sizeof(PASSENGER_SYNC_DATA));

	if(pPlayer)	{
		if(psSync.VehicleID == 0xFFFF) {
			KickPlayer((BYTE)p->playerIndex);
			return;
		}
	    pPlayer->StorePassengerFullSyncData(&psSync);
	}
}

//----------------------------------------------------

void CNetGame::Packet_SpectatorSync(Packet *p)
{
	CPlayer * pPlayer = GetPlayerPool()->GetAt((BYTE)p->playerIndex);
	RakNet::BitStream bsSpectatorSync((PCHAR)p->data, p->length, false);

	if(GetGameState() != GAMESTATE_RUNNING) return;

	BYTE bytePacketID=0;
	SPECTATOR_SYNC_DATA spSync;
	
	bsSpectatorSync.Read(bytePacketID);
	bsSpectatorSync.Read((PCHAR)&spSync,sizeof(SPECTATOR_SYNC_DATA));

	if(pPlayer)	{
	    pPlayer->StoreSpectatorFullSyncData(&spSync);
	}
}

void CNetGame::Packet_TrailerSync(Packet *p)
{
	CPlayer * pPlayer = GetPlayerPool()->GetAt((BYTE)p->playerIndex);
	RakNet::BitStream bsTrailerSync((PCHAR)p->data, p->length, false);

	if(GetGameState() != GAMESTATE_RUNNING) return;

	BYTE bytePacketID=0;
	TRAILER_SYNC_DATA trSync;
	
	bsTrailerSync.Read(bytePacketID);
	bsTrailerSync.Read((PCHAR)&trSync, sizeof(TRAILER_SYNC_DATA));

	if(pPlayer)	{
	    pPlayer->StoreTrailerFullSyncData(&trSync);
	}
	
}

//----------------------------------------------------

void CNetGame::Packet_StatsUpdate(Packet *p)
{
	RakNet::BitStream bsStats((PCHAR)p->data, p->length, false);
	CPlayerPool *pPlayerPool = GetPlayerPool();
	BYTE bytePlayerID = (BYTE)p->playerIndex;
	int iMoney;
	WORD wAmmo;
	BYTE bytePacketID;

	bsStats.Read(bytePacketID);
	bsStats.Read(iMoney);
	bsStats.Read(wAmmo);

	if(pPlayerPool) {
		if(pPlayerPool->GetSlotState(bytePlayerID)) {
			pPlayerPool->SetPlayerMoney(bytePlayerID,iMoney);
			pPlayerPool->SetPlayerAmmo(bytePlayerID, (DWORD)wAmmo);
			pPlayerPool->GetAt(bytePlayerID)->SetCurrentWeaponAmmo((DWORD)wAmmo);
		}
	}	
}

//----------------------------------------------------

void CNetGame::Packet_WeaponsUpdate(Packet *p)
{
	RakNet::BitStream bsData((PCHAR)p->data, p->length, false);
	CPlayerPool *pPlayerPool = GetPlayerPool();
	BYTE bytePlayerID = (BYTE)p->playerIndex;

	BYTE byteLength = (p->length - 1) / 4; // Should be number of changed weapons
	
	//printf("Original: %d New: %d", p->length, byteLength);
	
	BYTE byteIndex;
	BYTE byteWeapon;
	WORD wordAmmo;
	
	if (pPlayerPool)
	{
		//printf("1");
		if (pPlayerPool->GetSlotState(bytePlayerID))
		{
			//printf("2");
			CPlayer* pPlayer = pPlayerPool->GetAt(bytePlayerID);
			bsData.Read(byteIndex); // Dump the first byte, we don't need it
			while (byteLength)
			{
				//printf("3");
				bsData.Read(byteIndex);
				bsData.Read(byteWeapon);
				bsData.Read(wordAmmo);
				//printf("\n%u %u %i", byteIndex, byteWeapon, wordAmmo);
				if (byteIndex < (BYTE)13)
				{
					//printf("\n%d %d %d", byteIndex, byteWeapon, wordAmmo);
					pPlayer->m_dwSlotAmmo[byteIndex] = (DWORD)wordAmmo;
					pPlayer->m_byteSlotWeapon[byteIndex] = byteWeapon;
				}
				byteLength--;
			}
		}
	}
}

//----------------------------------------------------

void CNetGame::Packet_NewIncomingConnection(Packet* packet)
{
	//logprintf("Incoming Connection: %d (%d)\n", packet->length, packet->playerId);
	
	in_addr in;
	in.s_addr = packet->playerId.binaryAddress;
	logprintf("Incomming connection: %s:%u",inet_ntoa(in),packet->playerId.port);
}

//----------------------------------------------------

void CNetGame::Packet_DisconnectionNotification(Packet* packet)
{
	m_pPlayerPool->Delete((BYTE)packet->playerIndex,1);
}

//----------------------------------------------------

void CNetGame::Packet_ConnectionLost(Packet* packet)
{
	m_pPlayerPool->Delete((BYTE)packet->playerIndex,0);	
}

//----------------------------------------------------

void CNetGame::Packet_ModifiedPacket(Packet* packet)
{
	//logprintf("Packet was modified, sent by id: %d, ip: %s", 
					//(unsigned int)packet->playerIndex, packet->playerId.ToString());
}

//----------------------------------------------------

void CNetGame::Packet_RemotePortRefused(Packet* packet)
{
	logprintf("Remote Port Refused for Player: %u\n", packet->playerIndex);
}

//----------------------------------------------------

void CNetGame::Packet_InGameRcon(Packet* packet)
{
	CPlayer* pPlayer = GetPlayerPool()->GetAt((BYTE)packet->playerIndex);
	if (pPlayer)
	{
		DWORD dwCmdLen;
		memcpy(&dwCmdLen, &packet->data[1], 4);
		char* cmd = (char*)malloc(dwCmdLen+1);
		memcpy(cmd, &packet->data[5], dwCmdLen);
		cmd[dwCmdLen] = 0;

		if(GetPlayerPool()->IsAdmin((BYTE)packet->playerIndex))
		{
			if (strlen(cmd) == 0)
			{
				SendClientMessage(packet->playerId, 0xFFFFFFFF, "You forgot the RCON command!");
				return;
			}
			logprintf("RCON (In-Game): Player [%s] sent command: %s", GetPlayerPool()->GetPlayerName((BYTE)packet->playerIndex), cmd);
			byteRconUser = (BYTE)packet->playerIndex;
			pConsole->Execute(cmd);
			byteRconUser = INVALID_ID;
		} else {
			char* szTemp = strtok(cmd, " ");
			if (szTemp)
			{
				if (stricmp(szTemp, "login") == 0)
				{
					szTemp = strtok(NULL, " ");
					if (szTemp)
					{
						if(strcmp(szTemp, pConsole->GetStringVariable("rcon_password")) == 0)
						{
							GetPlayerPool()->SetAdmin((BYTE)packet->playerIndex);						
							logprintf("RCON (In-Game): Player #%d (%s) has logged in.", packet->playerIndex, GetPlayerPool()->GetPlayerName((BYTE)packet->playerIndex));
							SendClientMessage(packet->playerId, 0xFFFFFFFF,"SERVER: You are logged in as admin.");
						} else {
							logprintf("RCON (In-Game): Player #%d (%s) <%s> failed login.", packet->playerIndex, GetPlayerPool()->GetPlayerName((BYTE)packet->playerIndex), szTemp, pConsole->GetStringVariable("rcon_password"));
							SendClientMessage(packet->playerId, 0xFFFFFFFF,"SERVER: Bad admin password. Repeated attempts will get you banned.");
						}
					}
				}
			}
		}

		free(cmd);
	}
}

//----------------------------------------------------

void CNetGame::ProcessClientJoin(BYTE bytePlayerID)
{

	// Perform all init operations.
	if(GetGameState() == GAMESTATE_RUNNING) {
		m_pVehiclePool->InitForPlayer(bytePlayerID); // give them all the existing vehicles
		m_pPickupPool->InitForPlayer(bytePlayerID); // give them all the existing pickups
		m_pObjectPool->InitForPlayer(bytePlayerID); // give them all the existing map objects
		InitGameForPlayer(bytePlayerID);
		m_pPlayerPool->InitPlayersForPlayer(bytePlayerID); // let them know who's on the server
		m_pPlayerPool->InitSpawnsForPlayer(bytePlayerID); // spawn any existing players for them
		m_pMenuPool->ResetPlayer(bytePlayerID); // Mark them as requiring menus
		
		// Inform them of their VW as it doesn't actually work if called from OnPlayerConnect
		// The server is updated but they're not connected fully so don't get it, so resend it
		BYTE byteVW = m_pPlayerPool->GetPlayerVirtualWorld(bytePlayerID);
		RakNet::BitStream bsData;
		bsData.Write(bytePlayerID); // player id
		bsData.Write(byteVW); // VW id
		m_pRak->RPC(RPC_ScrSetPlayerVirtualWorld, &bsData, HIGH_PRIORITY, RELIABLE, 0, UNASSIGNED_PLAYER_ID, true, false);
	} 
	else if(GetGameState() == GAMESTATE_RESTARTING)
	{
		InitGameForPlayer(bytePlayerID);
		m_pPlayerPool->InitPlayersForPlayer(bytePlayerID); // let them know who's on the server
		// Tell them that the world is currently restarting.
		RakNet::BitStream bsParams;
		PlayerID sender = m_pRak->GetPlayerIDFromIndex(bytePlayerID);
		m_pRak->RPC(RPC_GameModeRestart, &bsParams, HIGH_PRIORITY,
			RELIABLE,0,sender,false,false);
	}
	
	//GetGameLogic()->HandleClientJoin(bytePlayerID);
	//GetGameMode()->OnPlayerConnect(bytePlayerID);

}

void CNetGame::SendClientMessage(PlayerID pidPlayer, DWORD dwColor, char* szMessage, ...)
{
	va_list va;
	va_start(va, szMessage);
	char szBuffer[512] = { 0 };
	vsprintf(szBuffer, szMessage, va);
	va_end(va);

	RakNet::BitStream bsParams;
	DWORD dwStrLen = strlen(szBuffer);

	bsParams.Write(dwColor);
	bsParams.Write(dwStrLen);
	bsParams.Write(szBuffer, dwStrLen);
	GetRakServer()->RPC(RPC_ClientMessage, &bsParams, HIGH_PRIORITY, RELIABLE, 0, pidPlayer, false, false);
}

//----------------------------------------------------

void CNetGame::SendClientMessageToAll(DWORD dwColor, char* szMessage, ...)
{
	va_list va;
	va_start(va, szMessage);
	char szBuffer[512] = { 0 };
	vsprintf(szBuffer, szMessage, va);
	va_end(va);

	RakNet::BitStream bsParams;
	DWORD dwStrLen = strlen(szBuffer);

	bsParams.Write(dwColor);
	bsParams.Write(dwStrLen);
	bsParams.Write(szBuffer, dwStrLen);
	GetRakServer()->RPC(RPC_ClientMessage, &bsParams, HIGH_PRIORITY, RELIABLE, 0, UNASSIGNED_PLAYER_ID, true, false);
}

//----------------------------------------------------

void CNetGame::InitGameForPlayer(BYTE bytePlayerID)
{
	RakNet::BitStream bsInitGame;
	bool bLanMode = false;

	if(m_bLanMode) bLanMode = true;

	bsInitGame.Write(m_iSpawnsAvailable);
	bsInitGame.Write(bytePlayerID); // send them their own player ID so they know.
	bsInitGame.Write(m_bShowNameTags);
	bsInitGame.Write(m_bShowPlayerMarkers);
	bsInitGame.Write(m_bTirePopping);
	bsInitGame.Write(m_byteWorldTime);
	bsInitGame.Write(m_byteWeather);
	bsInitGame.Write(m_fGravity);
	bsInitGame.Write(bLanMode);
	bsInitGame.Write(m_iDeathDropMoney);
	bsInitGame.Write(pConsole->GetBoolVariable("instagib"));
	bsInitGame.Write(m_bZoneNames);
	bsInitGame.Write(m_bUseCJWalk);
	bsInitGame.Write(m_bAllowWeapons);
	bsInitGame.Write(m_bLimitGlobalChatRadius);
	bsInitGame.Write(m_fGlobalChatRadius);
	bsInitGame.Write(m_bStuntBonus);
	bsInitGame.Write(m_fNameTagDrawDistance);
	bsInitGame.Write(m_bDisableEnterExits);
	
	char* szHostName = pConsole->GetStringVariable("hostname");
	if(szHostName) {
		BYTE byteStrLen = strlen(szHostName);
		bsInitGame.Write(byteStrLen);
		bsInitGame.Write(szHostName, byteStrLen);
	} else {
		bsInitGame.Write((BYTE)0);
	}

	GetRakServer()->RPC(RPC_InitGame,&bsInitGame,HIGH_PRIORITY,RELIABLE,0,GetRakServer()->GetPlayerIDFromIndex(bytePlayerID),false,false);
}

//----------------------------------------------------

void CNetGame::SetWorldTime(BYTE byteHour)
{
	RakNet::BitStream bsTime;

	m_byteWorldTime = byteHour;
	bsTime.Write(m_byteWorldTime);
	GetRakServer()->RPC(RPC_WorldTime,&bsTime,HIGH_PRIORITY,RELIABLE,0,UNASSIGNED_PLAYER_ID,true,false);

	char szTime[256];
	sprintf(szTime, "%02d:%02d", m_byteWorldTime, 0);
	pConsole->SetStringVariable("worldtime", szTime);
}

void CNetGame::SetWeather(BYTE byteWeather)
{
	RakNet::BitStream bsWeather;
	m_byteWeather = byteWeather;
	bsWeather.Write(m_byteWeather);
	GetRakServer()->RPC(RPC_Weather, &bsWeather, HIGH_PRIORITY, RELIABLE, 0, UNASSIGNED_PLAYER_ID, true, false);
	
	char szWeather[128];
	sprintf(szWeather, "%d", m_byteWeather);
	
	pConsole->SetStringVariable("weather", szWeather);
}

//----------------------------------------------------

void CNetGame::SetGravity(float fGravity)
{
	m_fGravity =		fGravity;
	RakNet::BitStream	bsGravity;
	bsGravity.Write(m_fGravity);
		
	char szGravity[128];
	sprintf(szGravity, "%f", m_fGravity);

	pConsole->SetStringVariable("gravity", szGravity);
	GetRakServer()->RPC(RPC_ScrSetGravity, &bsGravity, HIGH_PRIORITY, RELIABLE, 0, UNASSIGNED_PLAYER_ID, true, false);
}

//----------------------------------------------------

void CNetGame::KickPlayer(BYTE byteKickPlayer)
{
	PlayerID plr = GetRakServer()->GetPlayerIDFromIndex(byteKickPlayer);

	if(byteKickPlayer < MAX_PLAYERS) {
		if(m_pPlayerPool->GetSlotState(byteKickPlayer)) {
			GetRakServer()->Kick(plr);
			m_pPlayerPool->Delete(byteKickPlayer,2);			
		}
	}
}

//----------------------------------------------------

void CNetGame::AddBan(char * nick, char * ip_mask, char * reason)
{
	const struct tm *tm;
	time_t now;
	now = time(NULL);
	tm = localtime(&now);
	char *s;
	s = new char[256];
	strftime(s, 256, "[%d/%m/%y | %H:%M:%S]", tm);

	m_pRak->AddToBanList(ip_mask);
	
	FILE * fileBanList = fopen("samp.ban","a");
	if(!fileBanList) return;
	
	fprintf(fileBanList,"%s %s %s - %s\n", ip_mask, s, nick, reason);
	fclose(fileBanList);

	delete [] s;
}

//----------------------------------------------------

void CNetGame::RemoveBan(char* ip_mask)
{
	m_pRak->RemoveFromBanList(ip_mask);
	
	FILE* fileBanList = fopen("samp.ban", "r");
	FILE* fileWriteList = fopen("samp.ban.temp", "w");
	if(!fileBanList || !fileWriteList) return;
	
	char line[256];
	char line2[256];
	char* ip;
	
	while(!feof(fileBanList))
	{
		if (fgets(line, 256, fileBanList))
		{
			strcpy(line2, line);
			ip = strtok(line, " \t");
			if (strcmp(ip_mask, ip) != 0)
			{
				fprintf(fileWriteList, "%s", line2);
			}
		}
	}
	fclose(fileBanList);
	fclose(fileWriteList);
	remove("samp.ban");
	rename("samp.ban.temp", "samp.ban");
}

//----------------------------------------------------

void CNetGame::LoadBanList()
{
	m_pRak->ClearBanList();

	FILE * fileBanList = fopen("samp.ban","r");

	if(!fileBanList) {
		return;
	}

	char tmpban_ip[256];

	while(!feof(fileBanList)) {
		if (fgets(tmpban_ip,256,fileBanList)) {
			int len = strlen(tmpban_ip);
			if (len > 0 && tmpban_ip[len - 1] == '\n')
				tmpban_ip[len - 1] = 0;
			len = strlen(tmpban_ip);
			if (len > 0 && tmpban_ip[len - 1] == '\r')
				tmpban_ip[len - 2] = 0;
			if (tmpban_ip[0] != 0 && tmpban_ip[0] != '#') {
				char *ban_ip = strtok(tmpban_ip, " \t");
				m_pRak->AddToBanList(ban_ip);
			}
		}
	}

	logprintf("");
	logprintf("Ban list");
	logprintf("--------");
	logprintf(" Loaded: samp.ban");
	logprintf("");

	fclose(fileBanList);
}

//----------------------------------------------------

int CNetGame::AddSpawn(PLAYER_SPAWN_INFO *pSpawnInfo)
{
	if (m_iSpawnsAvailable < MAX_SPAWNS)
	{
		memcpy(&m_AvailableSpawns[m_iSpawnsAvailable],pSpawnInfo,sizeof(PLAYER_SPAWN_INFO));
		return m_iSpawnsAvailable++;
	}
	return MAX_SPAWNS;
}

//----------------------------------------------------

void CNetGame::UpdateInstagib()
{
	RakNet::BitStream bsInstagib;
	bsInstagib.Write((BYTE)(pConsole->GetBoolVariable("instagib")?1:0));
	GetRakServer()->RPC(RPC_Instagib, &bsInstagib, HIGH_PRIORITY, RELIABLE, 0, UNASSIGNED_PLAYER_ID, true, false);
}

//----------------------------------------------------

const PCHAR CNetGame::GetWeaponName(int iWeaponID)
{
	switch(iWeaponID) { 
      case WEAPON_BRASSKNUCKLE: 
         return "Brass Knuckles"; 
      case WEAPON_GOLFCLUB: 
         return "Golf Club"; 
      case WEAPON_NITESTICK: 
         return "Nite Stick"; 
      case WEAPON_KNIFE: 
         return "Knife"; 
      case WEAPON_BAT: 
         return "Baseball Bat"; 
      case WEAPON_SHOVEL: 
         return "Shovel"; 
      case WEAPON_POOLSTICK: 
         return "Pool Cue"; 
      case WEAPON_KATANA: 
         return "Katana"; 
      case WEAPON_CHAINSAW: 
         return "Chainsaw"; 
      case WEAPON_DILDO: 
         return "Dildo"; 
      case WEAPON_DILDO2: 
         return "Dildo"; 
      case WEAPON_VIBRATOR: 
         return "Vibrator"; 
      case WEAPON_VIBRATOR2: 
         return "Vibrator"; 
      case WEAPON_FLOWER: 
         return "Flowers"; 
      case WEAPON_CANE: 
         return "Cane"; 
      case WEAPON_GRENADE: 
         return "Grenade"; 
      case WEAPON_TEARGAS: 
         return "Teargas"; 
      case WEAPON_COLT45: 
         return "Colt 45"; 
      case WEAPON_SILENCED: 
         return "Silenced Pistol"; 
      case WEAPON_DEAGLE: 
         return "Desert Eagle"; 
      case WEAPON_SHOTGUN: 
         return "Shotgun"; 
      case WEAPON_SAWEDOFF: 
         return "Sawn-off Shotgun"; 
      case WEAPON_SHOTGSPA: // wtf? 
         return "Combat Shotgun"; 
      case WEAPON_UZI: 
         return "UZI"; 
      case WEAPON_MP5: 
         return "MP5"; 
      case WEAPON_AK47: 
         return "AK47"; 
      case WEAPON_M4: 
         return "M4"; 
      case WEAPON_TEC9: 
         return "TEC9"; 
      case WEAPON_RIFLE: 
         return "Rifle"; 
      case WEAPON_SNIPER: 
         return "Sniper Rifle"; 
      case WEAPON_ROCKETLAUNCHER: 
         return "Rocket Launcher"; 
      case WEAPON_HEATSEEKER: 
         return "Heat Seaker"; 
      case WEAPON_FLAMETHROWER: 
         return "Flamethrower"; 
      case WEAPON_MINIGUN: 
         return "Minigun"; 
      case WEAPON_SATCHEL: 
         return "Satchel Explosives"; 
      case WEAPON_BOMB: 
         return "Bomb"; 
      case WEAPON_SPRAYCAN: 
         return "Spray Can"; 
      case WEAPON_FIREEXTINGUISHER: 
         return "Fire Extinguisher"; 
      case WEAPON_CAMERA: 
         return "Camera"; 
      case WEAPON_PARACHUTE: 
         return "Parachute"; 
      case WEAPON_VEHICLE: 
         return "Vehicle"; 
      case WEAPON_DROWN: 
         return "Drowned"; 
      case WEAPON_COLLISION: 
         return "Splat";
	}

	return "";
}

//----------------------------------------------------
// EOF