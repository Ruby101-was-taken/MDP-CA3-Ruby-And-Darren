#include "RoboCatServerPCH.hpp"
#include <iostream>
#include <cmath>
#include <chrono>

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
	mLobbyDuration(15.0f),
	race_finished_(false)
{

	GameObjectRegistry::sInstance->RegisterCreationFunction('RCAR', PlayerCarServer::StaticCreate);
	GameObjectRegistry::sInstance->RegisterCreationFunction('MOUS', StarServer::StaticCreate);
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
		Vector3(-2125.552246, -1200, 90),
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
		Vector3(-2234.680420,-1852.070435, 0),
		Vector3(-2029.098755,-2137.342773, 0),
		Vector3(-1855.768433,-2271.496582, 0),
		Vector3(-1717.521851,-2324.414551, 0),
		Vector3(-1476.128540,-2357.430420, 0),
		Vector3(-1213.550171,-2307.666016, 0),
		Vector3(-1007.104858,-2184.969238, 0),
		Vector3(-766.020081,-1400.545410, 0),
		Vector3(-692.179749,-1148.710327, 0),
		Vector3(-542.413574,-1011.674744, 0),
		Vector3(-376.563995,-916.677063, 0),
		Vector3(476.723816,-1280.629761, 0),
		Vector3(515.229614,-1460.536011, 0),
		Vector3(582.193665,-1633.229736, 0),
		Vector3(685.364136,-1898.500000, 0),
		Vector3(1887.681030,-1868.265991, 0),
		Vector3(1932.471436,-1736.036743, 0),
		Vector3(1993.636475,-1559.295410, 0),
		Vector3(1961.408325,-717.280640, 0),
		Vector3(1478.293579,-503.406952, 0),
		Vector3(1352.748901,-383.283478, 0),
		Vector3(1235.322998,-262.057709, 0),
		Vector3(1022.160034,-156.976685, 0),
		Vector3(851.064758,-240.847214, 0),
		Vector3(584.405884,-255.703293, 0),
		Vector3(485.643311,-94.883743, 0),
		Vector3(223.327713,100.773682, 0),
		Vector3(76.308487,-47.962566, 0),
		Vector3(-381.152161,-62.158356, 0),
		Vector3(-581.739990,25.626419, 0),
		Vector3(-760.916565,172.365143, 0),
		Vector3(-825.828796,334.838745, 0),
		Vector3(-801.066284,543.392212, 0),
		Vector3(-681.708313,686.935425, 0),
		Vector3(-516.197021,770.524048, 0),
		Vector3(372.579803,515.461853, 0),
		Vector3(411.556732,747.270142, 0),
		Vector3(628.348389,658.273743, 0),
		Vector3(870.313416,540.325439, 0),
		Vector3(946.104004,759.506042, 0),
		Vector3(1899.795166,624.380859, 0),
		Vector3(2202.031738,875.885742, 0),
		Vector3(2231.210205,1329.161499, 0),
		Vector3(2234.752930,1591.392700, 0),
		Vector3(2205.046387,1839.237427, 0),
		Vector3(857.970581,1941.727539, 0),
		Vector3(654.763245,1757.167358, 0),
		Vector3(471.286682,1565.449585, 0),
		Vector3(186.792160,1477.844849, 0),
		Vector3(-706.199646,2183.587646, 0),
		Vector3(-993.737244,2325.242432, 0),
		Vector3(-1855.265503,2134.057373, 0),
		Vector3(-1992.076172,1950.339233, 0),
		Vector3(-2069.976807,1727.225342, 0)
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

	//NetworkManagerServer::sInstance->RespawnCats(); // TODO: Check how this affects performance

	Engine::DoFrame();

	NetworkManagerServer::sInstance->SendOutgoingPackets();
	// Darren Meidl - D00255479 - Lobby handling
	if (!NetworkManagerServer::sInstance)
		return;

	// If game-over detected, ensure lobby opens (once) and start timer
	if (ScoreBoardManager::sInstance && ScoreBoardManager::sInstance->GetIsGameOver()) {
		const auto p1 = std::chrono::system_clock::now();
		int now = std::chrono::duration_cast<std::chrono::seconds>(p1.time_since_epoch()).count();
		if (!race_finished_) {
			race_finished_ = true;
			race_end_time_ = now + kResultsTimer;
		}
		else if(now >= race_end_time_) {
			NetworkManagerServer::sInstance->SetIsInLobby(true); // allow joins

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
	// If lobby is open, perform client join logic
	if (NetworkManagerServer::sInstance && NetworkManagerServer::sInstance->IsInLobby())
	{
		int playerId = inClientProxy->GetPlayerId();
		ScoreBoardManager::sInstance->AddEntry(playerId, inClientProxy->GetName(), inClientProxy->GetPlayerColour());
		SpawnCarForPlayer(playerId, inClientProxy->GetPlayerColour());
	}
}

void Server::SpawnCarForPlayer(int inPlayerId, const Vector3& colour)
{
	PlayerCarPtr cat = std::static_pointer_cast<PlayerCar>(GameObjectRegistry::sInstance->CreateGameObject('RCAR'));
	cat->SetColor(colour);
	cat->SetPlayerId(inPlayerId);

	// Ruby White - D00255322
	float spawn_y = -239 + (120*(inPlayerId-1));
	cat->SetLocation(Vector3((inPlayerId%2==0)? -2240 : -2070, spawn_y, 0.f));

	// inform car of checkpoint count and race length
	cat->SetTotalCheckpoints(checkpoint_count_);
	cat->SetLapsToWin(4);
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
