#include "RoboCatServerPCH.hpp"
#include <iostream>
#include <cmath>

bool Server::StaticInit()
{
	s_instance.reset(new Server());

	// initialize RaceManager & Level Manager before handling clients
	RaceManager::StaticInit();
	LevelManager::StaticInit();

	return true;
}

Server::Server() :
	mLobbyOpenStartTime(0.f),
	mLobbyDuration(15.0f)
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
	string portString = SaveFileUtilities::GetPortFromFile();
	uint16_t port = stoi(portString);

	return NetworkManagerServer::StaticInit(port);
}


namespace
{
	const int kNumCheckpoints = 6;
	const float kCheckpointRadius = 160.f;

	// Ruby White - D00255322
	//places for checkpoits to spawn. Z indicates rotation cuz why not
	const vector<Vector3> checkpoints = {
		Vector3(-2125.552246, -421.344971, 90),
		Vector3(-1466.522827, -2102.799316, 0),
		Vector3(51.324478, -1018.3812871, 0),
		Vector3(1409.942139, -2075.799316, 0),
		Vector3(2100.924805, -1308.708008, 90),
		Vector3(318.778168, -150.539062, -30),
		Vector3(498.528992, 750.646057, 0),
		Vector3(2100.924805, 1559, 90),
		Vector3(1169.368042, 2109.872070, 0),
		Vector3(57.127823, 1640.833984, 0),
		Vector3(-1383.010132, 2065.174805, 0)
	};

	const vector<Vector3> stars = {
		Vector3(-2233.294922,-1851.933228, 0),
		Vector3(-2174.682373,-2002.284546, 0),
		Vector3(-2010.644775,-2244.369629, 0),
		Vector3(-1860.846436,-2313.968018, 0),
		Vector3(-1705.111938,-2377.123291, 0),
		Vector3(-1558.105835,-2393.663330, 0),
		Vector3(-1101.033325,-1937.104980, 0),
		Vector3(-991.427368,-1751.270752, 0),
		Vector3(-803.191284,-1396.502563, 0),
		Vector3(-748.541321,-1293.435303, 0),
		Vector3(-698.294800,-1198.672852, 0),
		Vector3(611.904358,-1737.300171, 0),
		Vector3(642.368591,-1856.134888, 0),
		Vector3(672.785156,-1964.581787, 0),
		Vector3(758.382324,-2123.711426, 0),
		Vector3(1670.224609,-2016.929077, 0),
		Vector3(1899.711914,-1815.526855, 0),
		Vector3(2025.338135,-1554.193237, 0),
		Vector3(1579.370361,-514.878601, 0),
		Vector3(1480.263428,-362.062927, 0),
		Vector3(1301.985352,-258.936279, 0),
		Vector3(1014.572205,-355.959259, 0),
		Vector3(930.919861,-249.705338, 0),
		Vector3(763.913757,-87.836014, 0),
		Vector3(428.944092,-142.116104, 0),
		Vector3(-568.960266,74.278107, 0),
		Vector3(-769.785767,148.638123, 0),
		Vector3(-866.232544,391.171967, 0),
		Vector3(-844.221069,637.625427, 0),
		Vector3(-661.673035,719.276001, 0),
		Vector3(-504.343445,725.807007, 0),
		Vector3(38.679264,748.849487, 0),
		Vector3(538.168335,725.931091, 0),
		Vector3(1258.572876,690.544922, 0),
		Vector3(2221.179688,868.516357, 0),
		Vector3(2286.036377,1256.964966, 0),
		Vector3(2283.503174,1641.413574, 0),
		Vector3(2190.668945,1920.870728, 0),
		Vector3(1887.311279,2103.394775, 0),
		Vector3(1531.497192,2183.992676, 0),
		Vector3(621.138367,1750.798584, 0),
		Vector3(509.717072,1446.943726, 0),
		Vector3(222.285202,1373.663696, 0),
		Vector3(-76.196808,1421.484863, 0),
		Vector3(-334.100586,1531.814087, 0),
		Vector3(-1619.398560,2219.198975, 0),
		Vector3(-1912.579346,1988.562134, 0),
		Vector3(-2019.555298,1671.722778, 0)
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
	// NOTE: automatic start of the next round after the lobby timer has expired has been removed.
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

	//// If lobby is open, delay car spawning until the race starts
	//if (NetworkManagerServer::sInstance && !NetworkManagerServer::sInstance->IsInLobby())
	//{
	//	SpawnCarForPlayer(playerId, inClientProxy->GetPlayerColour());
	//}
	if (NetworkManagerServer::sInstance) {
		SpawnCarForPlayer(playerId, inClientProxy->GetPlayerColour());
	}
	// Register player with RaceManager
	if (RaceManager::sInstance)
	{
		RaceManager::sInstance->AddPlayer(playerId);
	}
}

void Server::SpawnCarForPlayer(int inPlayerId, const Vector3& colour)
{
	// If a car for this player already exists, reuse it instead of spawning another
	PlayerCarPtr existing = GetCarForPlayer(inPlayerId);
	if (existing)
	{
		// reset state / reposition existing car rather than creating a duplicate
		existing->SetColor(colour);
		existing->SetPlayerId(inPlayerId);

		float spawn_y = -239 + (120*(inPlayerId-1));
		existing->SetLocation(Vector3((inPlayerId%2==0)? -2240 : -2070, spawn_y, 0.f));

		// inform car of checkpoint count and race length, reset progress
		existing->SetTotalCheckpoints(checkpoint_count_);
		existing->SetLapsToWin(3);
		existing->ResetRaceProgress();

		// ensure it's alive / not marked for removal
		existing->SetDoesWantToDie(false);

		// mark pose/state dirty so clients get updated
		if (NetworkManagerServer::sInstance)
		{
			NetworkManagerServer::sInstance->SetStateDirty(existing->GetNetworkId(), PlayerCar::ECRS_AllState);
		}

		return;
	}

	// otherwise create a fresh car
	PlayerCarPtr cat = std::static_pointer_cast<PlayerCar>(GameObjectRegistry::sInstance->CreateGameObject('RCAR'));
	cat->SetColor(colour);
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
