#include "RoboCatServerPCH.hpp"
#include <iostream>
#include <cmath>

bool Server::StaticInit()
{
	s_instance.reset(new Server());

	return true;
}

Server::Server()
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

}

void Server::HandleNewClient(ClientProxyPtr inClientProxy)
{

	int playerId = inClientProxy->GetPlayerId();

	ScoreBoardManager::sInstance->AddEntry(playerId, inClientProxy->GetName());
	SpawnCatForPlayer(playerId);
}

void Server::SpawnCatForPlayer(int inPlayerId)
{
	PlayerCarPtr cat = std::static_pointer_cast<PlayerCar>(GameObjectRegistry::sInstance->CreateGameObject('RCAT'));
	cat->SetColor(ScoreBoardManager::sInstance->GetEntry(inPlayerId)->GetColor());
	cat->SetPlayerId(inPlayerId);
	//gotta pick a better spawn location than this...
	cat->SetLocation(Vector3(600.f - static_cast<float>(inPlayerId), 400.f, 0.f));

	// inform cat of checkpoint count and race length
	cat->SetTotalCheckpoints(kNumCheckpoints);
	cat->SetLapsToWin(3);
	cat->ResetRaceProgress();
}

void Server::HandleLostClient(ClientProxyPtr inClientProxy)
{
	//kill client's cat
	//remove client from scoreboard
	int playerId = inClientProxy->GetPlayerId();

	ScoreBoardManager::sInstance->RemoveEntry(playerId);
	PlayerCarPtr cat = GetCatForPlayer(playerId);
	if (cat)
	{
		cat->SetDoesWantToDie(true);
	}
}

PlayerCarPtr Server::GetCatForPlayer(int inPlayerId)
{
	//run through the objects till we find the cat...
	//it would be nice if we kept a pointer to the cat on the clientproxy
	//but then we'd have to clean it up when the cat died, etc.
	//this will work for now until it's a perf issue
	const auto& gameObjects = World::sInstance->GetGameObjects();
	for (int i = 0, c = gameObjects.size(); i < c; ++i)
	{
		GameObjectPtr go = gameObjects[i];
		PlayerCar* cat = go->GetAsCar();
		if (cat && cat->GetPlayerId() == inPlayerId)
		{
			return std::static_pointer_cast<PlayerCar>(go);
		}
	}

	return nullptr;

}
