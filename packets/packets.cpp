//----------

bool RakPeer::Send( const char *data, const int length, PacketPriority priority, PacketReliability reliability, char orderingChannel, PlayerID playerId, bool broadcast )
{
#ifdef _DEBUG
	assert( data && length > 0 );
#endif

	if ( data == 0 || length < 0 )
		return false;

	if ( remoteSystemList == 0 || endThreads == true )
		return false;

	if ( broadcast == false && playerId == UNASSIGNED_PLAYER_ID )
		return false;

	if (broadcast==false && router && GetIndexFromPlayerID(playerId)==-1)
	{
		return router->Send(data, BYTES_TO_BITS(length), priority, reliability, orderingChannel, playerId);
	}
	else
	{
		SendBuffered(data, length*8, priority, reliability, orderingChannel, playerId, broadcast, RemoteSystemStruct::NO_ACTION);
	}

	return true;
}

// From telnet:

PlayerID TelnetTransport::HasLostConnection(void)
{
	PlayerID playerId;
	unsigned i;
	playerId=tcpInterface->HasLostConnection();
	if (playerId!=UNASSIGNED_PLAYER_ID)
	{
		for (i=0; i < remoteClients.Size(); i++)
		{
			if (remoteClients[i]->playerId==playerId)
			{
				delete remoteClients[i];
				remoteClients[i]=remoteClients[remoteClients.Size()-1];
				remoteClients.Del();
			}
		}
	}
	return playerId;
}

// TCP:

void TCPInterface::CloseConnection( PlayerID playerId )
{
	if (isStarted==false)
		return;
	if (playerId==UNASSIGNED_PLAYER_ID)
		return;
	PlayerID *id = requestedCloseConnections.WriteLock();
	*id=playerId;
	requestedCloseConnections.WriteUnlock();
}

void TCPInterface::DeallocatePacket( Packet *packet )
{
	assert(incomingMessages.CheckReadUnlockOrder(packet));
	delete [] packet->data;
	incomingMessages.ReadUnlock();
}
