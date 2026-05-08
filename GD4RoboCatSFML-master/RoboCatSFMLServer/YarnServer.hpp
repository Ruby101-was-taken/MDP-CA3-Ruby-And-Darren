class YarnServer : public Yarn
{
public:
	static GameObjectPtr	StaticCreate() { return NetworkManagerServer::sInstance->RegisterAndReturn(new YarnServer()); }
	void HandleDying() override;

	virtual bool		HandleCollisionWithCar(PlayerCar* inCat) override;

	virtual void Update() override;

protected:
	YarnServer();

private:
	float mTimeToDie;

};

