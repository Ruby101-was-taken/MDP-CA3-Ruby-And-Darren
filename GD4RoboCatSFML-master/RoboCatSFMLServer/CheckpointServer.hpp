// Darren Meidl - D000255479 - Entire class
class CheckpointServer : public Checkpoint 
{
public:
	static GameObjectPtr StaticCreate() { return NetworkManagerServer::sInstance->RegisterAndReturn(new CheckpointServer()); }
	void HandleDying() override;
	virtual bool HandleCollisionWithCar(PlayerCar* inCar) override;

protected:
	CheckpointServer();
};