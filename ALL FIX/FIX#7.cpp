// Keys:


BOOL HandleKeyPress(DWORD vKey)
{
	static int i = -1;
	switch(vKey)
	{
		case VK_F6: //  CHAT
			if(!pCmdWindow->isEnabled()) {
				pCmdWindow->Enable();
			} else {
				pCmdWindow->Disable();
			}
			break;

		case VK_F7: // ON/OFF CHAT
			pChatWindow->CycleMode();
			break;

		case VK_F8: // SCREENSHOT
			g_bTakeScreenshot = TRUE;
			break;

		case VK_F9: // DEATH WINDOW ON/OFF
			pDeathWindow->ToggleEnabled();
			break;
			
		case VK_F11: // CHANGE FONT
			pChangeFont//-> NO FUNC ? : D
			break;
			
  /*
  
    [.........................]
  
  */
  
  
static cell AMX_NATIVE_CALL n_SetVehicleNumberPlate(AMX *amx, cell *params)
{
	CHECK_PARAMS(2);

	if (pNetGame->GetVehiclePool()->GetSlotState((VEHICLEID)params[1]))
	{
		char *szInput;

		CHAR szPlate[9] = "";
		amx_StrParam(amx, params[2], szInput);

		/*
			Here it is possible to crash the client
		*/
		pNetGame->GetVehiclePool()->GetAt((VEHICLEID)params[1])->SetNumberPlate(szPlate);
		return 1;
	}
	return 0;
}
  
//-------------------------------------------------------------


// 0x504D4153 = "SAMP" as hex.
if(!pNetGame || (pNetGame->GetGameState() != GAMESTATE_RUNNING)) return 0;

//[...........]

			switch (data[10])
			{
				case 'p':	// ping
				{
					if (length == 15)
					{
						// ..HERE..
					}
					break;
				}
				
				
