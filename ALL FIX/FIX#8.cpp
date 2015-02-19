	// From AMX
br_extract_prefix (const char *path)
{
	char *end, *tmp, *result;

	br_return_val_if_fail (path != (char *) NULL, (char *) NULL);

	if (!*path) return strdup ("/");
	end = strrchr (path, '/');
	if (!end) return strdup (path);

	tmp = br_strndup ((char *) path, end - path);
	if (!*tmp)
	{
		free (tmp);
		return strdup ("/");
	}
	end = strrchr (tmp, '/');
	if (!end) return tmp;

	result = br_strndup (tmp, end - tmp);
	free (tmp);

	if (!*result)
	{
		free (result);
		result = strdup ("/");
	}

	return result;
}

	// RakNet
	
	
void NatPunchthrough::OnPunchthroughRequest(RakPeerInterface *peer, Packet *packet)
{
	if (allowFacilitation==false)
		return;

	PlayerID target;
	RakNet::BitStream inBitstream(packet->data, packet->length, false);
	RakNet::BitStream outBitstream;
	inBitstream.IgnoreBits(8);
	if (inBitstream.Read(target)==false)
		return;
	if (rakPeer->GetIndexFromPlayerID(target)==-1)
	{
		outBitstream.Write((unsigned char)ID_NAT_TARGET_NOT_CONNECTED);
		outBitstream.Write(target);
		rakPeer->Send(&outBitstream, HIGH_PRIORITY, RELIABLE, 0, packet->playerId, false);
	}
	else
	{
		// Remember this connection request
		NatPunchthrough::ConnectionRequest *connectionRequest = new NatPunchthrough::ConnectionRequest;
		connectionRequest->receiver=target;
		connectionRequest->facilitator=UNASSIGNED_PLAYER_ID;
		connectionRequest->passwordData = 0;
		connectionRequest->sender=packet->playerId;
		connectionRequest->passwordDataLength=0;
		connectionRequest->facilitatingConnection=true;
		connectionRequest->nextActionTime=RakNet::GetTime()+PING_INTERVAL;
		connectionRequest->pingCount=1;
		connectionRequest->timeoutTime=RakNet::GetTime()+30000;
		connectionRequestList.Insert(connectionRequest);

		rakPeer->Ping(connectionRequest->receiver);
		rakPeer->Ping(connectionRequest->sender);
	}
}

bool ConnectionGraph::IsNewRemoteConnection(const PlayerIdAndGroupId &conn1, const PlayerIdAndGroupId &conn2,RakPeerInterface *peer)
{
	if (graph.HasConnection(conn1,conn2)==false &&
		subscribedGroups.HasData(conn1.groupId) &&
		subscribedGroups.HasData(conn2.groupId) &&
		(peer->GetIndexFromPlayerID(conn1.playerId)==-1 || peer->GetIndexFromPlayerID(conn2.playerId)==-1))
	{
		PlayerID externalId1, externalId2;
		externalId1=peer->GetExternalID(conn1.playerId);
		externalId2=peer->GetExternalID(conn2.playerId);
		return (externalId1!=conn1.playerId && externalId1!=conn2.playerId &&
			externalId2!=conn1.playerId && externalId2!=conn2.playerId);
	}
	return false;
}

// PATCHES
	// the mem
	int x=0;
	while(x!=600) {
		PedModelsMemory[x].func_tbl = 0x85BDC0;
		memset(PedModelsMemory[x].data,0,64);
		x++;
	}
