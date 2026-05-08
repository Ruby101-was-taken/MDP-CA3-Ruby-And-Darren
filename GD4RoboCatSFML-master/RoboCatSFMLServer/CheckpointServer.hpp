class CheckpointServer : public Checkpoint 
{
public:
	static GameObjectPtr StaticCreate() { return NetworkManagerServer::sInstance->RegisterAndReturn(new CheckpointServer()); }

	void HandleDying() override;

protected:
	CheckpointServer();
};