//----------------------------------------------------

void CRemotePlayer::StoreOnFootFullSyncData(ONFOOT_SYNC_DATA *pofSync)
{
	if(GetTickCount() < m_dwWaitForEntryExitAnims) return;

	m_pCurrentVehicle = NULL;
	memcpy(&m_ofSync,pofSync,sizeof(ONFOOT_SYNC_DATA));
	m_fReportedHealth = (float)pofSync->byteHealth;
	m_fReportedArmour = (float)pofSync->byteArmour;
	m_byteUpdateFromNetwork = UPDATE_TYPE_ONFOOT;
	
	SetState(PLAYER_STATE_ONFOOT);
}

//----------------------------------------------------

void CRemotePlayer::StoreInCarFullSyncData(INCAR_SYNC_DATA *picSync)
{	
	if(GetTickCount() < m_dwWaitForEntryExitAnims) return;
	memcpy(&m_icSync,picSync,sizeof(INCAR_SYNC_DATA));

	m_VehicleID = picSync->VehicleID;
	m_byteSeatID = 0;
	m_pCurrentVehicle = pNetGame->GetVehiclePool()->GetAt(m_VehicleID);

	m_fReportedHealth = (float)picSync->bytePlayerHealth;
	m_fReportedArmour =	(float)picSync->bytePlayerArmour;
	m_byteUpdateFromNetwork = UPDATE_TYPE_INCAR;
	
	SetState(PLAYER_STATE_DRIVER);
}

//----------------------------------------------------

void CRemotePlayer::StorePassengerFullSyncData(PASSENGER_SYNC_DATA *ppsSync)
{
	if(GetTickCount() < m_dwWaitForEntryExitAnims) return;
	memcpy(&m_psSync,ppsSync,sizeof(PASSENGER_SYNC_DATA));

	m_VehicleID = ppsSync->VehicleID;
	m_byteSeatID = ppsSync->byteSeatFlags & 127;
	m_pCurrentVehicle = pNetGame->GetVehiclePool()->GetAt(m_VehicleID);

	m_fReportedHealth = (float)ppsSync->bytePlayerHealth;
	m_fReportedArmour = (float)ppsSync->bytePlayerArmour;
	m_byteUpdateFromNetwork = UPDATE_TYPE_PASSENGER;
	
	SetState(PLAYER_STATE_PASSENGER);
}