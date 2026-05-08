class Checkpoint;
class RoboCat : public GameObject
{
public:
	CLASS_IDENTIFICATION('RCAT', GameObject)

	enum ECatReplicationState
	{
		ECRS_Pose = 1 << 0,
		ECRS_Color = 1 << 1,
		ECRS_PlayerId = 1 << 2,
		ECRS_Health = 1 << 3,

		ECRS_AllState = ECRS_Pose | ECRS_Color | ECRS_PlayerId | ECRS_Health
	};


	static	GameObject* StaticCreate() { return new RoboCat(); }

	virtual uint32_t GetAllStateMask()	const override { return ECRS_AllState; }

	virtual	RoboCat* GetAsCat() override { return this; }

	virtual void Update() override;

	void ProcessInput(float inDeltaTime, const InputState& inInputState);
	void SimulateMovement(float inDeltaTime);

	void ProcessCollisions();
	void ProcessCollisionsWithScreenWalls();

	void		SetPlayerId(uint32_t inPlayerId) { mPlayerId = inPlayerId; }
	uint32_t	GetPlayerId()						const { return mPlayerId; }

	void			SetVelocity(const Vector3& inVelocity) { mVelocity = inVelocity; }
	const Vector3& GetVelocity()						const { return mVelocity; }

	virtual uint32_t	Write(OutputMemoryBitStream& inOutputStream, uint32_t inDirtyState) const override;

	// Darren Meidl - D000255479 - Checkpoint + lap logic
	void OnCheckpointPassed(Checkpoint* inCheckpoint);
	void ResetRaceProgress();
	void SetTotalCheckpoints(int inTotal) { mTotalCheckpoints = inTotal; }
	void SetLapsToWin(int inLaps) { mLapsToWin = inLaps; }
	int GetCurrentLap() const { return mCurrentLap; }
	int GetCurrentCheckpointIndex() const { return mCurrentCheckpointIndex; }
	bool IsRaceFinished() const { return mRaceFinished; }

protected:
	RoboCat();

private:

	void	AdjustVelocityByThrust(float inDeltaTime);

	Vector3				mVelocity;


	float				mMaxLinearSpeed;
	float				mMaxRotationSpeed;
	float				mAcceleration;
	float				mReverseAccelScale;
	float				mLinearDrag;
	float				mGrip;	

	//bounce fraction when hitting various things
	float				mWallRestitution;
	float				mCatRestitution;


	uint32_t			mPlayerId;

protected:

	///move down here for padding reasons...

	float				mLastMoveTimestamp;

	float				mThrustDir;
	int					mHealth;

	bool				mIsShooting;

private:
	// checkpoint & lap tracking
	int					mCurrentLap;
	int					mCurrentCheckpointIndex; // -1 == none yet
	int					mLapsToWin;
	int					mTotalCheckpoints;
	bool				mRaceFinished;
};

typedef shared_ptr< RoboCat >	RoboCatPtr;