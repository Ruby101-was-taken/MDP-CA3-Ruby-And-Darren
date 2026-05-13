class ServerTrack : public Track {
public:
	static GameObjectPtr	StaticCreate() { return NetworkManagerServer::sInstance->RegisterAndReturn(new ServerTrack()); }

protected:
	ServerTrack();
};
