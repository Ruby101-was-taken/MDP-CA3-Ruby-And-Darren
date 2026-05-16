#include "RoboCatServerPCH.hpp"

NetworkManagerServer* NetworkManagerServer::sInstance;


NetworkManagerServer::NetworkManagerServer() :
	mNewPlayerId(1),
	mNewNetworkId(1),
	mTimeBetweenStatePackets(0.033f),
	mClientDisconnectTimeout(3.f),
	mIsInLobby(true)
{
}

bool NetworkManagerServer::StaticInit(uint16_t inPort)
{
	sInstance = new NetworkManagerServer();
	return sInstance->Init(inPort);
}

void NetworkManagerServer::HandleConnectionReset(const SocketAddress& inFromAddress)
{
	//just dc the client right away...
	auto it = mAddressToClientMap.find(inFromAddress);
	if (it != mAddressToClientMap.end())
	{
		HandleClientDisconnected(it->second);
	}
}

void NetworkManagerServer::ProcessPacket(InputMemoryBitStream& inInputStream, const SocketAddress& inFromAddress)
{
	//try to get the client proxy for this address
	//pass this to the client proxy to process
	auto it = mAddressToClientMap.find(inFromAddress);
	if (it == mAddressToClientMap.end())
	{
		//didn't find one? it's a new cilent..is the a HELO? if so, create a client proxy...
		HandlePacketFromNewClient(inInputStream, inFromAddress);
	}
	else
	{
		ProcessPacket((*it).second, inInputStream);
	}
}


void NetworkManagerServer::ProcessPacket(ClientProxyPtr inClientProxy, InputMemoryBitStream& inInputStream)
{
	//remember we got a packet so we know not to disconnect for a bit
	inClientProxy->UpdateLastPacketTime();

	uint32_t	packetType;
	inInputStream.Read(packetType);
	switch (packetType)
	{
	case kHelloCC:
		//need to resend welcome...
		SendWelcomePacket(inClientProxy);
		break;
	case kInputCC:
		if (inClientProxy->GetDeliveryNotificationManager().ReadAndProcessState(inInputStream))
		{
			// If lobby is open, ignore movement input from non-host clients.
			if (mIsInLobby && inClientProxy->GetPlayerId() != 1)
			{
				Logging::Log("NetworkManagerServer::ProcessPacket", "Ignoring input packet from player " + std::to_string(inClientProxy->GetPlayerId()) + " while lobby is open");
			}
			else
			{
				HandleInputPacket(inClientProxy, inInputStream);
			}
		}
		break;
	case kStartRaceCC:
		// Only allow player 1 (host) to start the race.
		if (inClientProxy->GetPlayerId() == 1)
		{
			if (mIsInLobby)
			{
				mIsInLobby = false;
				// Reset race state, repopulate players and spawn cars immediately
				if (RaceManager::sInstance)
				{
					RaceManager::sInstance->Reset();
					std::vector<int> connected = GetConnectedPlayerIds();
					for (int pid : connected)
					{
						RaceManager::sInstance->AddPlayer(static_cast<uint32_t>(pid));
					}
					for (int pid : connected)
					{
						static_cast<Server*>(Engine::s_instance.get())->SpawnCarForPlayer(pid, mPlayerIdToClientMap[pid]->GetPlayerColour());
					}
				}
			}
		}
		else
		{
			LOG("Non-host attempted to start race (player %d) - ignored", inClientProxy->GetPlayerId());
		}
		break;
	default:
		LOG("Unknown packet type received from %s", inClientProxy->GetSocketAddress().ToString().c_str());
		break;
	}
}


void NetworkManagerServer::HandlePacketFromNewClient(InputMemoryBitStream& inInputStream, const SocketAddress& inFromAddress)
{
	//read the beginning- is it a hello?
	uint32_t	packetType;
	inInputStream.Read(packetType);
	if (packetType == kHelloCC)
	{
		// Darren Meidl - D00255479
		// If we're currently in a game, reject new join attempts.
		if (!mIsInLobby)
		{
			LOG("Rejecting join from %s - game in progress", inFromAddress.ToString().c_str());
			return; // simply ignore the hello; client will retry or time out.
		}

		//read the name
		string name;
		inInputStream.Read(name);
		Vector3 playerColour;
		inInputStream.Read(playerColour);
		ClientProxyPtr newClientProxy = std::make_shared< ClientProxy >(inFromAddress, name, mNewPlayerId++);
		newClientProxy->SetPlayerColour(playerColour);
		mAddressToClientMap[inFromAddress] = newClientProxy;
		mPlayerIdToClientMap[newClientProxy->GetPlayerId()] = newClientProxy;

		//tell the server about this client, spawn a car, etc...
		static_cast<Server*> (Engine::s_instance.get())->HandleNewClient(newClientProxy);

		//and welcome the client...
		SendWelcomePacket(newClientProxy);

		//and now init the replication manager with everything we know about!
		for (const auto& pair : m_network_id_to_game_object_map)
		{
			newClientProxy->GetReplicationManagerServer().ReplicateCreate(pair.first, pair.second->GetAllStateMask());
		}
	}
	else
	{
		//bad incoming packet from unknown client- we're under attack!!
		LOG("Bad incoming packet from unknown client at socket %s", inFromAddress.ToString().c_str());
	}
}

void NetworkManagerServer::SendWelcomePacket(ClientProxyPtr inClientProxy)
{
	OutputMemoryBitStream welcomePacket;

	welcomePacket.Write(kWelcomeCC);
	welcomePacket.Write(inClientProxy->GetPlayerId());

	LOG("Server Welcoming, new client '%s' as player %d", inClientProxy->GetName().c_str(), inClientProxy->GetPlayerId());

	SendPacket(welcomePacket, inClientProxy->GetSocketAddress());
}

void NetworkManagerServer::RespawnCats()
{
	for (auto it = mAddressToClientMap.begin(), end = mAddressToClientMap.end(); it != end; ++it)
	{
		ClientProxyPtr clientProxy = it->second;

		clientProxy->RespawnCarIfNecessary();
	}
}

void NetworkManagerServer::SendOutgoingPackets()
{
	//let's send a client a state packet whenever their move has come in...
	for (auto it = mAddressToClientMap.begin(), end = mAddressToClientMap.end(); it != end; ++it)
	{
		ClientProxyPtr clientProxy = it->second;
		//process any timed out packets while we're going through the list
		clientProxy->GetDeliveryNotificationManager().ProcessTimedOutPackets();

		// Send state either when they have new moves or when we're in lobby so clients get lobby/replication updates
		if (clientProxy->IsLastMoveTimestampDirty() || mIsInLobby)
		{
			SendStatePacketToClient(clientProxy);
		}
	}
}

void NetworkManagerServer::UpdateAllClients()
{
	for (auto it = mAddressToClientMap.begin(), end = mAddressToClientMap.end(); it != end; ++it)
	{
		//process any timed out packets while we're going throug hthe list
		it->second->GetDeliveryNotificationManager().ProcessTimedOutPackets();

		SendStatePacketToClient(it->second);
	}
}

void NetworkManagerServer::SendStatePacketToClient(ClientProxyPtr inClientProxy)
{
	//build state packet
	OutputMemoryBitStream	statePacket;

	//it's state!
	statePacket.Write(kStateCC);

	InFlightPacket* ifp = inClientProxy->GetDeliveryNotificationManager().WriteState(statePacket);

	WriteLastMoveTimestampIfDirty(statePacket, inClientProxy);

	statePacket.Write(mIsInLobby);

	AddScoreBoardStateToPacket(statePacket);

	ReplicationManagerTransmissionData* rmtd = new ReplicationManagerTransmissionData(&inClientProxy->GetReplicationManagerServer());
	inClientProxy->GetReplicationManagerServer().Write(statePacket, rmtd);
	ifp->SetTransmissionData('RPLM', TransmissionDataPtr(rmtd));

	SendPacket(statePacket, inClientProxy->GetSocketAddress());

}

void NetworkManagerServer::WriteLastMoveTimestampIfDirty(OutputMemoryBitStream& inOutputStream, ClientProxyPtr inClientProxy)
{
	//first, dirty?
	bool isTimestampDirty = inClientProxy->IsLastMoveTimestampDirty();
	inOutputStream.Write(isTimestampDirty);
	if (isTimestampDirty)
	{
		inOutputStream.Write(inClientProxy->GetUnprocessedMoveList().GetLastMoveTimestamp());
		inClientProxy->SetIsLastMoveTimestampDirty(false);
	}
}

//should we ask the server for this? or run through the world ourselves?
void NetworkManagerServer::AddWorldStateToPacket(OutputMemoryBitStream& inOutputStream)
{
	const auto& gameObjects = World::sInstance->GetGameObjects();

	//now start writing objects- do we need to remember how many there are? we can check first...
	inOutputStream.Write(gameObjects.size());

	for (GameObjectPtr gameObject : gameObjects)
	{
		inOutputStream.Write(gameObject->GetNetworkId());
		inOutputStream.Write(gameObject->GetClassId());
		gameObject->Write(inOutputStream, 0xffffffff);

	}
}

void NetworkManagerServer::AddScoreBoardStateToPacket(OutputMemoryBitStream& inOutputStream)
{
	ScoreBoardManager::sInstance->Write(inOutputStream);
}


int NetworkManagerServer::GetNewNetworkId()
{
	int toRet = mNewNetworkId++;
	if (mNewNetworkId < toRet)
	{
		LOG("Network ID Wrap Around!!! You've been playing way too long...", 0);
	}

	return toRet;

}

void NetworkManagerServer::HandleInputPacket(ClientProxyPtr inClientProxy, InputMemoryBitStream& inInputStream)
{
	uint32_t moveCount = 0;
	Move move;
	inInputStream.Read(moveCount, 2);

	for (; moveCount > 0; --moveCount)
	{
		if (move.Read(inInputStream))
		{
			if (inClientProxy->GetUnprocessedMoveList().AddMoveIfNew(move))
			{
				inClientProxy->SetIsLastMoveTimestampDirty(true);
			}
		}
	}
}

ClientProxyPtr NetworkManagerServer::GetClientProxy(int inPlayerId) const
{
	auto it = mPlayerIdToClientMap.find(inPlayerId);
	if (it != mPlayerIdToClientMap.end())
	{
		return it->second;
	}

	return nullptr;
}

void NetworkManagerServer::CheckForDisconnects()
{
	vector< ClientProxyPtr > clientsToDC;

	float minAllowedLastPacketFromClientTime = Timing::sInstance.GetTimef() - mClientDisconnectTimeout;
	for (const auto& pair : mAddressToClientMap)
	{
		if (pair.second->GetLastPacketFromClientTime() < minAllowedLastPacketFromClientTime)
		{
			//can't remove from map while in iterator, so just remember for later...
			clientsToDC.push_back(pair.second);
		}
	}

	for (ClientProxyPtr client : clientsToDC)
	{
		HandleClientDisconnected(client);
	}
}

void NetworkManagerServer::HandleClientDisconnected(ClientProxyPtr inClientProxy)
{
	mPlayerIdToClientMap.erase(inClientProxy->GetPlayerId());
	mAddressToClientMap.erase(inClientProxy->GetSocketAddress());
	static_cast<Server*> (Engine::s_instance.get())->HandleLostClient(inClientProxy);

	//was that the last client? if so, bye!
	if (mAddressToClientMap.empty())
	{
		Engine::s_instance->SetShouldKeepRunning(false);
	}
}

void NetworkManagerServer::RegisterGameObject(GameObjectPtr inGameObject)
{
	//assign network id
	int newNetworkId = GetNewNetworkId();
	inGameObject->SetNetworkId(newNetworkId);

	//add mapping from network id to game object
	m_network_id_to_game_object_map[newNetworkId] = inGameObject;

	//tell all client proxies this is new...
	for (const auto& pair : mAddressToClientMap)
	{
		pair.second->GetReplicationManagerServer().ReplicateCreate(newNetworkId, inGameObject->GetAllStateMask());
	}
}


void NetworkManagerServer::UnregisterGameObject(GameObject* inGameObject)
{
	int networkId = inGameObject->GetNetworkId();
	m_network_id_to_game_object_map.erase(networkId);

	//tell all client proxies to STOP replicating!
	//tell all client proxies this is new...
	for (const auto& pair : mAddressToClientMap)
	{
		pair.second->GetReplicationManagerServer().ReplicateDestroy(networkId);
	}
}

void NetworkManagerServer::SetStateDirty(int inNetworkId, uint32_t inDirtyState)
{
	//tell everybody this is dirty
	for (const auto& pair : mAddressToClientMap)
	{
		pair.second->GetReplicationManagerServer().SetStateDirty(inNetworkId, inDirtyState);
	}
}

std::vector<int> NetworkManagerServer::GetConnectedPlayerIds() const
{
	std::vector<int> ids;
	ids.reserve(mPlayerIdToClientMap.size());
	for (const auto& pair : mPlayerIdToClientMap)
	{
		ids.push_back(pair.first);
	}
	return ids;
}