// Unique ID client
// For this shit : http://ugbase.eu/releases-6/gpci-unbanner-%28cleo!!!%29/

/*
	native gpci(playerid, serial[], maxlen); // Get Serial-Number player + unique ID
	//
	native BanGPCIPlayer(playerid, serial[] .... serialid)
	//
*/

// forward OnPlayerGPCIBanned(playerid, serialid);
int CGameMode::OnPlayerGPCIBanned(cell playerid, cell serialid)
{
        CHECK_INIT();

        int idx;
        cell ret = 0;

        if (!amx_FindPublic(&m_amx, "OnPlayerGPCIBanned", &idx))
        {
                amx_Push(&m_amx, amx_ftoc( serialid ) );
                amx_Push(&m_amx, playerid);
                amx_Exec(&m_amx, &ret, idx);
        }
        return (int)ret;
}


