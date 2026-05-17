class StarClient : public Star
{
public:
	static GameObjectPtr StaticCreate() { return GameObjectPtr(new StarClient()); }

	virtual void Update() override;
	virtual bool HandleCollisionWithCar(PlayerCar* inCat) override;
protected:
	StarClient();

private:
	SpriteComponentPtr sprite_component_;
};
