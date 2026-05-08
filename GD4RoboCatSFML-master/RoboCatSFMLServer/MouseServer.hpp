class MouseServer : public Mouse
{
public:
	static GameObjectPtr	StaticCreate() { return NetworkManagerServer::sInstance->RegisterAndReturn(new MouseServer()); }
	void HandleDying() override;
	virtual bool HandleCollisionWithCat(PlayerCar* inCat) override;

protected:
	MouseServer();
};