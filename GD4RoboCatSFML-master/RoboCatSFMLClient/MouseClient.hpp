class MouseClient : public Mouse
{
public:
	static GameObjectPtr StaticCreate() { return GameObjectPtr(new MouseClient()); }

	virtual void Update() override;
protected:
	MouseClient();

private:
	SpriteComponentPtr sprite_component_;
};
