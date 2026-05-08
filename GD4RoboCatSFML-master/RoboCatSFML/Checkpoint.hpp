// Darren Meidl - D00255479 - Entire class
class Checkpoint : public GameObject {
public:
	CLASS_IDENTIFICATION('CHKP', Checkpoint)

	Checkpoint() : GameObject(), mIndex(0) {}
	Checkpoint(int inIndex) : GameObject(), mIndex(inIndex) {}

	static std::shared_ptr<GameObject> CreateSharedInstance() { return std::make_shared<Checkpoint>(); }

	void SetIndex(int inIndex) { mIndex = inIndex; }
	int GetIndex() const { return mIndex; }

	// When a cat collides with a checkpoint, notify the cat.
	// Return false to skip the physics collision response (no bounce).
	virtual bool HandleCollisionWithCat(RoboCat* inCat) override;

	// replication
	enum ECheckpointReplicationState
	{
		ECRS_Pose = 1 << 0,
		ECRS_Index = 1 << 1,

		ECRS_AllState = ECRS_Pose | ECRS_Index
	};

	virtual uint32_t GetAllStateMask() const override { return ECRS_AllState; }
	virtual uint32_t Write(OutputMemoryBitStream& inOutputStream, uint32_t inDirtyState) const override;
	virtual void Read(InputMemoryBitStream& inInputStream) override;

private:
	int mIndex;
};