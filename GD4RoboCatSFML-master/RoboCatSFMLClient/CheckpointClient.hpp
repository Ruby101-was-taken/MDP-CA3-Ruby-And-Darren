// Darren Meidl - D000255479 - Entire class
class CheckpointClient : public Checkpoint 
{
public:
	static GameObjectPtr StaticCreate() { return GameObjectPtr(new CheckpointClient()); }

protected:
	CheckpointClient();
	virtual bool HandleCollisionWithCar(PlayerCar* inCar) override;

public:
	void SetPassed(bool inPassed);
	void SetNext(bool inIsNext);

private:
	SpriteComponentPtr mSpriteComponent;
	bool mPassed = false;
	bool mIsNext = false;

	const float visualScaleX = 1.0f;
	const float visualScaleY = 5.0f;
};