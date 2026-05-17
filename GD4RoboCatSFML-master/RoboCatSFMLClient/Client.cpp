#include "RoboCatClientPCH.hpp"
#include <filesystem>

bool Client::StaticInit()
{
	// Create the Client pointer first because it initializes SDL
	Client* client = new Client();
	InputManager::StaticInit();

	WindowManager::StaticInit();
	FontManager::StaticInit();
	TextureManager::StaticInit();
	SoundManager::StaticInit();
	RenderManager::StaticInit();

	SoundManager::sInstance->PlayMusic("../Assets/Sound/Music/Theme/Lobby.wav");


	GameObjectPtr go;
	go = GameObjectRegistry::sInstance->CreateGameObject('TRCK');
	Vector3 track_location(0, 0, 0);
	go->SetLocation(track_location);
	

	HUD::StaticInit();

	s_instance.reset(client);


	Logging::LogInit();
	Logging::Log("Client", "Log Initialised");

	return true;
}

Client::Client()
{
	GameObjectRegistry::sInstance->RegisterCreationFunction('RCAR', PlayerCarClient::StaticCreate);
	GameObjectRegistry::sInstance->RegisterCreationFunction('MOUS', StarClient::StaticCreate);
	GameObjectRegistry::sInstance->RegisterCreationFunction('YARN', YarnClient::StaticCreate);
	GameObjectRegistry::sInstance->RegisterCreationFunction('CHKP', CheckpointClient::StaticCreate);
	GameObjectRegistry::sInstance->RegisterCreationFunction('TRCK', ClientTrack::StaticCreate);

	if (not SaveFileUtilities::CheckIfFolderExists("Data")) {
		std::filesystem::create_directories("Data");
	}

	string destination = SaveFileUtilities::GetAddressFromFile() + ":" + SaveFileUtilities::GetPortFromFile();
	string name = SaveFileUtilities::GetUserNameFromFile();

	Logging::Log("Client", destination);

	SocketAddressPtr serverAddress = SocketAddressFactory::CreateIPv4FromString(destination);

	NetworkManagerClient::StaticInit(*serverAddress, name);
	LevelManager::StaticInit();


	//NetworkManagerClient::sInstance->SetSimulatedLatency(0.0f);
}



void Client::DoFrame()
{
	InputManager::sInstance->Update();

	Engine::DoFrame();

	NetworkManagerClient::sInstance->ProcessIncomingPackets();

	RenderManager::sInstance->Render();

	NetworkManagerClient::sInstance->SendOutgoingPackets();

	SoundManager::sInstance->RemoveStoppedSounds();
}

void Client::HandleEvent(sf::Event& p_event)
{
	switch (p_event.type)
	{
	case sf::Event::KeyPressed:
		// Darren Meidl - D00255479
		if (p_event.key.code == sf::Keyboard::S) // Detect 'S' (start-race) key
		{
			if (NetworkManagerClient::sInstance)
            {
                bool lobby = NetworkManagerClient::sInstance->IsLobbyOpen();
                int pid = NetworkManagerClient::sInstance->GetPlayerId();
                LOG("S pressed: IsLobbyOpen=%d, PlayerId=%d", lobby ? 1 : 0, pid);

                if (lobby)
                {
					// only host (player id 1) should be able to hit start
                    if (pid == 1) 
                        NetworkManagerClient::sInstance->SendStartRacePacket();
                }
            }
		}
		InputManager::sInstance->HandleInput(EIA_Pressed, p_event.key.code);
		break;
	case sf::Event::KeyReleased:
		InputManager::sInstance->HandleInput(EIA_Released, p_event.key.code);
		break;
	default:
		break;
	}
}

bool Client::PollEvent(sf::Event& p_event)
{
	return WindowManager::sInstance->pollEvent(p_event);
}


