class CheckpointClient : public Checkpoint 
{
public:
	static GameObjectPtr StaticCreate() { return GameObjectPtr(new CheckpointClient()); }

protected:
	CheckpointClient();

private:
	SpriteComponentPtr mSpriteComponent;
};