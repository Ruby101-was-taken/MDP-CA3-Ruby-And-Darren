enum ECarControlType
{
	ESCT_Human,
	ESCT_AI
};

class PlayerCarServer : public PlayerCar
{
public:
	static GameObjectPtr	StaticCreate() { return NetworkManagerServer::sInstance->RegisterAndReturn(new PlayerCarServer()); }
	virtual void HandleDying() override;

	virtual void Update() override;

	void SetCatControlType(ECarControlType inCarControlType) { mCarControlType = inCarControlType; }

	void TakeDamage(int inDamagingPlayerId);

	// Ruby White - D00255322
	void IncreaseTopSpeed() override;
protected:
	PlayerCarServer();

private:

	void HandleShooting();

	ECarControlType	mCarControlType;


	float		mTimeOfNextShot;
	float		mTimeBetweenShots;

};

