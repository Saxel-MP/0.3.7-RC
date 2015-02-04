void CPlayer::StorePassengerFullSyncData(PASSENGER_SYNC_DATA *ppsSync)
{
        m_VehicleID  = ppsSync->VehicleID;
       
        CFilterScripts * pFilterScripts = pNetGame->GetFilterScripts();
        CGameMode * pGameMode = pNetGame->GetGameMode();
 
        memcpy(&m_psSync,ppsSync,sizeof(PASSENGER_SYNC_DATA));
        UpdatePosition(m_psSync.vecPos.X,m_psSync.vecPos.Y,m_psSync.vecPos.Z);
 
        m_fHealth = (float)m_psSync.bytePlayerHealth;
        m_fArmour = (float)m_psSync.bytePlayerArmour;
 
        m_psSync.byteCurrentWeapon = CheckWeapon(m_psSync.byteCurrentWeapon);
       
        m_byteUpdateFromNetwork = UPDATE_TYPE_PASSENGER;
 
        /*
                HERE...
        */
