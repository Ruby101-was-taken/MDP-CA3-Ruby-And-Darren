class Track : public GameObject {
public:
	CLASS_IDENTIFICATION('TRCK', Track)

	static	GameObject* StaticCreate() { return new Track(); }

protected:
	Track();

};

