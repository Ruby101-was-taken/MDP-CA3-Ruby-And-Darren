#include "RoboCatServerPCH.hpp"
#include <iostream>
#include <cmath>

bool Server::StaticInit()
{
	s_instance.reset(new Server());

	// initialize RaceManager before handling clients
	RaceManager::StaticInit();

	return true;
}

Server::Server() :
	mLobbyOpenStartTime(0.f),
	mLobbyDuration(5.0f)
{

	GameObjectRegistry::sInstance->RegisterCreationFunction('RCAR', PlayerCarServer::StaticCreate);
	GameObjectRegistry::sInstance->RegisterCreationFunction('MOUS', MouseServer::StaticCreate);
	GameObjectRegistry::sInstance->RegisterCreationFunction('YARN', YarnServer::StaticCreate);
	GameObjectRegistry::sInstance->RegisterCreationFunction('CHKP', CheckpointServer::StaticCreate);

	InitNetworkManager();

	// Setup latency
	float latency = 0.0f;
	string latencyString = StringUtils::GetCommandLineArg(2);
	if (!latencyString.empty())
	{
		latency = stof(latencyString);
	}
	NetworkManagerServer::sInstance->SetSimulatedLatency(latency);
}


int Server::Run()
{
	SetupWorld();

	Logging::LogInit();
	Logging::Log("Server", "Log Initialised");

	return Engine::Run();
}

bool Server::InitNetworkManager()
{
	string portString = StringUtils::GetCommandLineArg(1);
	uint16_t port = stoi(portString);

	return NetworkManagerServer::StaticInit(port);
}


namespace
{
	const int kNumCheckpoints = 6;
	const float kCheckpointRadius = 40.f;

	void CreateRandomMice(int inMouseCount)
	{
		Vector3 mouseMin(100.f, 100.f, 0.f);
		Vector3 mouseMax(1180.f, 620.f, 0.f);
		GameObjectPtr go;

		//make a mouse somewhere- where will these come from?
		for (int i = 0; i < inMouseCount; ++i)
		{
			go = GameObjectRegistry::sInstance->CreateGameObject('MOUS');
			Vector3 mouseLocation = RoboMath::GetRandomVector(mouseMin, mouseMax);
			go->SetLocation(mouseLocation);
		}
	}

	void CreateCheckpoints(int inCount)
	{
		// Layout checkpoints in a straight horizontal line centered in the level.
		// This makes it easy to run through them in order.
		Vector3 center(600.f, 400.f, 0.f);

		// Total span of the checkpoints along X. Adjust if you want them more/less spread out.
		const float totalSpan = 600.f;

		// If only one checkpoint, place it exactly at center.
		float startX = center.mX;
		float spacing = 0.f;
		if (inCount > 1)
		{
			startX = center.mX - totalSpan * 0.5f;
			spacing = totalSpan / float(inCount - 1);
		}

		for (int i = 0; i < inCount; ++i)
		{
			GameObjectPtr go = GameObjectRegistry::sInstance->CreateGameObject('CHKP');
			Vector3 loc(startX + spacing * float(i), center.mY, 0.f);
			go->SetLocation(loc);
			go->SetCollisionRadius(kCheckpointRadius);

			// set index on checkpoint
			std::shared_ptr<Checkpoint> cp = std::static_pointer_cast<Checkpoint>(go);
			cp->SetIndex(i);
		}
	}
}


void Server::SetupWorld()
{
	//spawn some random mice
	CreateRandomMice(10);

	// spawn checkpoints for the race
	CreateCheckpoints(kNumCheckpoints);

	//spawn more random mice!
	//CreateRandomMice(10);
}

void Server::DoFrame()
{
	NetworkManagerServer::sInstance->ProcessIncomingPackets();

	NetworkManagerServer::sInstance->CheckForDisconnects();

	NetworkManagerServer::sInstance->RespawnCats();

	Engine::DoFrame();

	NetworkManagerServer::sInstance->SendOutgoingPackets();
	// Darren Meidl - D00255479 - Lobby handling triggered by round end
	// When a round finishes we open the lobby for mLobbyDuration seconds to allow joins
	// After that window the lobby is closed and the next round starts (no joins allowed mid-game)
	if (!NetworkManagerServer::sInstance)
		return;

	// If game-over detected, ensure lobby opens (once) and start timer
	if (ScoreBoardManager::sInstance && ScoreBoardManager::sInstance->GetIsGameOver())
	{
		float now = Timing::sInstance.GetFrameStartTime();

		// Open lobby when we first notice game-over.
		if (mLobbyOpenStartTime == 0.f)
		{
			NetworkManagerServer::sInstance->SetIsInLobby(true); // allow joins
			mLobbyOpenStartTime = now;
		}
		else
		{
			// If lobby window expired, close lobby and start next round.
			if ((now - mLobbyOpenStartTime) >= mLobbyDuration)
			{
				// Close lobby to prevent mid-game joins
				NetworkManagerServer::sInstance->SetIsInLobby(false);

				// Reset per-round state so the next race can start cleanly.
				if (RaceManager::sInstance)
				{
					RaceManager::sInstance->Reset();

					// repopulate active players in the race manager
					std::vector<int> connected = NetworkManagerServer::sInstance->GetConnectedPlayerIds();
					for (int pid : connected)
					{
						RaceManager::sInstance->AddPlayer(static_cast<uint32_t>(pid));
					}
					// Spawn a car for any connected player that doesn't currently have one
					for (int pid : connected)
					{
						if (!GetCarForPlayer(pid))
							SpawnCarForPlayer(pid);
					}
				}

				// clear lobby timer so we can detect next round's game-over anew
				mLobbyOpenStartTime = 0.f;
			}
		}
	}
	else
	{
		// no game-over; ensure lobby timer cleared
		mLobbyOpenStartTime = 0.f;
	}
}

void Server::HandleNewClient(ClientProxyPtr inClientProxy)
{
	int playerId = inClientProxy->GetPlayerId();
	ScoreBoardManager::sInstance->AddEntry(playerId, inClientProxy->GetName());

	// If lobby is open, delay car spawning until the race starts
	if (NetworkManagerServer::sInstance && !NetworkManagerServer::sInstance->IsInLobby())
	{
		SpawnCarForPlayer(playerId);
	}
	else
	{
		LOG("Player %d joined during lobby; delaying car spawn until race start", playerId);
	}
	// Register player with RaceManager
	if (RaceManager::sInstance)
	{
		RaceManager::sInstance->AddPlayer(playerId);
	}
}

void Server::SpawnCarForPlayer(int inPlayerId)
{
	PlayerCarPtr cat = std::static_pointer_cast<PlayerCar>(GameObjectRegistry::sInstance->CreateGameObject('RCAR'));
	cat->SetColor(ScoreBoardManager::sInstance->GetEntry(inPlayerId)->GetColor());
	cat->SetPlayerId(inPlayerId);
	//gotta pick a better spawn location than this...
	cat->SetLocation(Vector3(600.f - static_cast<float>(inPlayerId), 400.f, 0.f));

	// inform car of checkpoint count and race length
	cat->SetTotalCheckpoints(kNumCheckpoints);
	cat->SetLapsToWin(3);
	cat->ResetRaceProgress();
}

void Server::HandleLostClient(ClientProxyPtr inClientProxy)
{
	//kill client's car
	//remove client from scoreboard
	int playerId = inClientProxy->GetPlayerId();

	ScoreBoardManager::sInstance->RemoveEntry(playerId);
	PlayerCarPtr cat = GetCarForPlayer(playerId);
	if (cat)
	{
		cat->SetDoesWantToDie(true);
	}

	// Unregister from RaceManager
	if (RaceManager::sInstance)
	{
		RaceManager::sInstance->RemovePlayer(playerId);
	}
}

PlayerCarPtr Server::GetCarForPlayer(int inPlayerId)
{
	//run through the objects till we find the car...
	const auto& gameObjects = World::sInstance->GetGameObjects();
	for (int i = 0, c = gameObjects.size(); i < c; ++i)
	{
		GameObjectPtr go = gameObjects[i];
		PlayerCar* car = go->GetAsCar();
		if (car && car->GetPlayerId() == inPlayerId)
		{
			return std::static_pointer_cast<PlayerCar>(go);
		}
	}

	return nullptr;

}
