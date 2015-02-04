native ToggleHUDComponent(componentid)
native ToggleHUDComponentForPlayer(playerid, componentid)
native SetRadioStation(playerid, station);
native RemoveButtonSpawn();
native SetPlayerStrength(playerid, strength);
native GetPlayerStrength(playerid);
/*
        GetPlayerResolution[...]
*/
 
Callbacks :
 
// forward OnExplosionCreated(playerid, Float:x, Float:y, Float:z, Float:radius);
int CGameMode::OnExplosionCreated(cell playerid, cell float posX, cell float posY, cell float posZ, cell float radius)
{
        CHECK_INIT();
 
        int idx;
        cell ret = 0;
 
        if (!amx_FindPublic(&m_amx, "OnExplosionCreated", &idx))
        {
                amx_Push(&m_amx, amx_ftoc( radius ) );
                amx_Push(&m_amx, amx_ftoc( posZ ) );
                amx_Push(&m_amx, amx_ftoc( posY ) );
                amx_Push(&m_amx, amx_ftoc( posX ) );
                amx_Push(&m_amx, playerid);
                amx_Exec(&m_amx, &ret, idx);
        }
        return (int)ret;
}
 
//=============================
 
// forward OnVirtualKeyStateChange(playerid, newkeys, oldkeys);
int CGameMode::OnVirtualKeyStateChange(cell playerid, cell newkeys, cell oldkeys)
{
        CHECK_INIT();
 
        int idx;
        cell ret = 0;
 
        if (!amx_FindPublic(&m_amx, "OnVirtualKeyStateChange", &idx))
        {
            	amx_Push(&m_amx, amx_ftoc( oldkeys ) );
                amx_Push(&m_amx, amx_ftoc( newkeys ) );
                amx_Push(&m_amx, amx_ftoc( playerid ) );
                amx_Exec(&m_amx, &ret, idx);
        }
        return (int)ret;
}