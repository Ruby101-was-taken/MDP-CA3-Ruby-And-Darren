class PlayerCarClient : public PlayerCar
{
public:
	static	GameObjectPtr	StaticCreate() { return GameObjectPtr(new PlayerCarClient()); }

	virtual void Update();
	virtual void	HandleDying() override;

	virtual void	Read(InputMemoryBitStream& inInputStream) override;

	void DoClientSidePredictionAfterReplicationForLocalCat(uint32_t inReadState);
	void DoClientSidePredictionAfterReplicationForRemoteCat(uint32_t inReadState);

	void OnCompleteLap() override;
	void OnCompleteRace() override;
	void SetLapsToWin(int inLaps) override;
protected:
	PlayerCarClient();


private:
	void InterpolateClientSidePrediction(float inOldRotation, const Vector3& inOldLocation, const Vector3& inOldVelocity, bool inIsForRemoteCat);
	float				mTimeLocationBecameOutOfSync;
	float				mTimeVelocityBecameOutOfSync;

	SpriteComponentPtr	mSpriteComponent;
};

