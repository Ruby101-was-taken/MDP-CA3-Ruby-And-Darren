class Checkpoint;
class PlayerCar : public GameObject
{
public:
	CLASS_IDENTIFICATION('RCAR', GameObject)

	enum ECarReplicationState
	{
		ECRS_Pose = 1 << 0,
		ECRS_Color = 1 << 1,
		ECRS_PlayerId = 1 << 2,
		ECRS_Health = 1 << 3,
		ECRS_Speed = 1 << 4,
		ECRS_Checkpoints = 1 << 5,

		ECRS_AllState = ECRS_Pose | ECRS_Color | ECRS_PlayerId | ECRS_Health | ECRS_Speed | ECRS_Checkpoints
	};


	static	GameObject* StaticCreate() { return new PlayerCar(); }

	virtual uint32_t GetAllStateMask()	const override { return ECRS_AllState; }

	virtual	PlayerCar* GetAsCar() override { return this; }

	virtual void Update() override;

	void ProcessInput(float inDeltaTime, const InputState& inInputState);
	void SimulateMovement(float inDeltaTime);

	void ProcessCollisions();
	void ProcessCollisionsWithLevel();

	void		SetPlayerId(uint32_t inPlayerId) { mPlayerId = inPlayerId; }
	uint32_t	GetPlayerId()						const { return mPlayerId; }

	void			SetVelocity(const Vector3& inVelocity) { mVelocity = inVelocity; }
	const Vector3& GetVelocity()						const { return mVelocity; }

	virtual uint32_t	Write(OutputMemoryBitStream& inOutputStream, uint32_t inDirtyState) const override;

	// Darren Meidl - D000255479 - Checkpoint + lap logic
	void OnCheckpointPassed(Checkpoint* inCheckpoint);
	void ResetRaceProgress();
	void SetLapsToWin(int inLaps) { mLapsToWin = inLaps; }
	int GetCurrentLap() const { return mCurrentLap; }
	int GetCurrentCheckpointIndex() const { return mCurrentCheckpointIndex; }
	bool IsRaceFinished() const { return mRaceFinished; }
	int GetLapsToWin() const { return mLapsToWin; }

	// Ruby White - D00255322
	virtual void SetTotalCheckpoints(int in_total);
	virtual void IncreaseTopSpeed();
	void OnCompleteLap();
protected:
	PlayerCar();

private:

	void	AdjustVelocityByThrust(float inDeltaTime);

	Vector3	mVelocity;


	float mMaxRotationSpeed;
	float mAcceleration;
	float mReverseAccelScale;
	float mLinearDrag;
	float mGrip;

	// Steering smoothing + speed-based steering scale
	float mCurrentSteer;	// current applied steering (-1..1)
	float mMinSteerScale;	// minimum steering scale at top speed (0..1)

	//bounce fraction when hitting various things
	float mWallRestitution;
	float mCarRestitution;


	// Ruby White - D00255322
	float star_speed_increase_;
	int stars_;
	int max_stars_;

	uint32_t mPlayerId;

protected:

	float mMaxLinearSpeed;
	///move down here for padding reasons...

	float mLastMoveTimestamp;

	float mThrustDir;
	int mHealth;

	bool mIsShooting;

private:
	// checkpoint & lap tracking
	int mCurrentLap;
	int mCurrentCheckpointIndex; // -1 == none yet
	int mLapsToWin;
	int total_checkpoints_;
	bool mRaceFinished;
};

typedef shared_ptr< PlayerCar >	PlayerCarPtr;