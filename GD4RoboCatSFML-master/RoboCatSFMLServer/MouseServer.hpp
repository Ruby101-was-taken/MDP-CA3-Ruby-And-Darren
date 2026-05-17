class StarServer : public Star
{
public:
	static GameObjectPtr	StaticCreate() { return NetworkManagerServer::sInstance->RegisterAndReturn(new StarServer()); }
	void HandleDying() override;
	virtual bool HandleCollisionWithCar(PlayerCar* inCat) override;

protected:
	StarServer();

	void Respawn() override;
};