// Darren Meidl - D000255479 - Entire class
class CheckpointClient : public Checkpoint 
{
public:
	static GameObjectPtr StaticCreate() { return GameObjectPtr(new CheckpointClient()); }

protected:
	CheckpointClient();

	// HandleCollisionWithCat is overridden on client to provide visual feedback when a checkpoint is passed.
	virtual bool HandleCollisionWithCat(RoboCat* inCat) override;

public:
	// Set the visual "passed" state (changes sprite texture).
	void SetPassed(bool inPassed);

private:
	SpriteComponentPtr mSpriteComponent;
	bool mPassed = false;
};