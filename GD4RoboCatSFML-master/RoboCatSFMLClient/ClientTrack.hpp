class ClientTrack : public Track {
public:
	static GameObjectPtr StaticCreate() { return GameObjectPtr(new ClientTrack()); }

protected:
	ClientTrack();

private:
	SpriteComponentPtr sprite_component_;
};
