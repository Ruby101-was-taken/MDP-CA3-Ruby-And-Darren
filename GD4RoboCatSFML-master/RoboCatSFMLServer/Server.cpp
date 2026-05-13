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
	// Ruby White - D00255322
	GameObjectRegistry::sInstance->RegisterCreationFunction('TRCK', ServerTrack::StaticCreate);

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

	// Ruby White - D00255322
	//places for checkpoits to spawn. Z indicates rotation cuz why not
	const vector<Vector3> checkpoints = {
		Vector3(-2125.552246, -421.344971, 90),
		Vector3(-1466.522827, -2102.799316, 0),
		Vector3(51.324478, -1018.3812871, 0),
		Vector3(1409.942139, -2075.799316, 0),
		Vector3(2100.924805, -1308.708008, 90),
		Vector3(318.778168, -93.539062, -30),
		Vector3(498.528992, 750.646057, 0),
		Vector3(2100.924805, 1559, 90),
		Vector3(1169.368042, 2109.872070, 0),
		Vector3(57.127823, 1640.833984, 0),
		Vector3(-1383.010132, 2135.174805, 0)
	};

	const vector<Vector3> stars = {
		Vector3(-2151.022949,-796.765137, 0),
		Vector3(-2145.224365,-1063.186401, 0),
		Vector3(-2112.547607,-1647.859009, 0),
		Vector3(-1713.181396,-2091.355713, 0),
		Vector3(-1128.787598,-2065.548096, 0),
		Vector3(-754.482971,-1594.193604, 0),
		Vector3(-416.820282,-1103.754272, 0),
		Vector3(151.486084,-976.750122, 0),
		Vector3(614.482727,-1323.291748, 0),
		Vector3(838.261963,-1895.145386, 0),
		Vector3(1329.267090,-2176.183350, 0),
		Vector3(1914.383301,-2024.205933, 0),
		Vector3(2212.397461,-1540.753296, 0),
		Vector3(2157.127441,-912.124512, 0),
		Vector3(1794.699219,-447.860138, 0),
		Vector3(1215.443604,-315.310303, 0),
		Vector3(626.994629,-164.481873, 0),
		Vector3(34.708496,-87.057114, 0),
		Vector3(-474.525879,213.981979, 0),
		Vector3(-317.743500,587.991516, 0),
		Vector3(260.299133,639.385376, 0),
		Vector3(896.876343,614.390320, 0),
		Vector3(1528.331299,629.404053, 0),
		Vector3(2088.906738,874.417664, 0),
		Vector3(2200.448242,1497.971924, 0),
		Vector3(1853.668213,2001.300049, 0),
		Vector3(1236.186401,2121.086182, 0),
		Vector3(636.281311,1978.076172, 0),
		Vector3(191.292435,1518.769409, 0),
		Vector3(-357.734497,1703.224976, 0),
		Vector3(-805.512268,2152.127441, 0),
		Vector3(-1418.191406,2289.562500, 0)
	};

	void CreateRandomMice(int inMouseCount)
	{
		Vector3 mouseMin(100.f, 100.f, 0.f);
		Vector3 mouseMax(1180.f, 620.f, 0.f);
		GameObjectPtr go;

		for (Vector3 vect : stars) {

			GameObjectPtr go = GameObjectRegistry::sInstance->CreateGameObject('MOUS');
			go->SetLocation(vect);
		}

	}

	int CreateCheckpoints(int inCount)
	{
		int i = 0;
		for (Vector3 vect : checkpoints) {

			GameObjectPtr go = GameObjectRegistry::sInstance->CreateGameObject('CHKP');
			go->SetLocation(vect);
			go->SetCollisionRadius(kCheckpointRadius);
			go->SetRotation(vect.mZ);
			go->SetScale(5);

			// set index on checkpoint
			std::shared_ptr<CheckpointServer> cp = std::static_pointer_cast<CheckpointServer>(go);
			cp->SetIndex(i++);
			Logging::Log("Server", "i: " + std::to_string(i));
		}

		return checkpoints.size();
	}
}


void Server::SetupWorld()
{
	//spawn some random mice
	CreateRandomMice(10);

	// spawn checkpoints for the race
	checkpoint_count_ = CreateCheckpoints(checkpoint_count_);
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

	// Ruby White - D00255322
	float spawn_y = -239 + (120*(inPlayerId-1));
	cat->SetLocation(Vector3((inPlayerId%2==0)? -2240 : -2070, spawn_y, 0.f));

	// inform car of checkpoint count and race length
	cat->SetTotalCheckpoints(checkpoint_count_);
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
