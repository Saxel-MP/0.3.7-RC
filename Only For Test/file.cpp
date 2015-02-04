//----------------------------------------

void CNetGame::Packet_AimSync(Packet *p)
{
	RakNet::BitStream bsAimSync((PCHAR)p->data, p->length, false);
	AIM_SYNC_DATA aimSync;
	BYTE bytePacketID=0;
	BYTE bytePlayerID=0;

	if(GetGameState() != GAMESTATE_CONNECTED) return;

	bsAimSync.Read(bytePacketID);
	bsAimSync.Read(bytePlayerID);
	bsAimSync.Read((PCHAR)&aimSync,sizeof(AIM_SYNC_DATA));
}


//----------------------------------------

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow)
{
	InitSettings();

	pNetGame = new CNetGame(tSettings.szConnectHost,atoi(tSettings.szConnectPort),
				tSettings.szNickName,tSettings.szConnectPass);

	while(1) {
		pNetGame->Process();
		Sleep(200);

		if( pNetGame->GetGameState() == GAMESTATE_CONNECTED &&
			!pNetGame->GetRakClient()->IsConnected() ) {
				break;
		}
	}
	return 0;
}

//------------------------------------------
