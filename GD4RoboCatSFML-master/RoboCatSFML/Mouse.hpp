class Mouse : public GameObject
{
public:
	CLASS_IDENTIFICATION('MOUS', GameObject)

	enum EMouseReplicationState
	{
		EMRS_Pose = 1 << 0,
		EMRS_Color = 1 << 1,
		EMRS_Active = 1 << 2,

		EMRS_AllState = EMRS_Pose | EMRS_Color | EMRS_Active
	};

	static	GameObject* StaticCreate() { return new Mouse(); }

	virtual uint32_t	GetAllStateMask()	const override { return EMRS_AllState; }

	virtual uint32_t	Write(OutputMemoryBitStream& inOutputStream, uint32_t inDirtyState) const override;
	virtual void		Read(InputMemoryBitStream& inInputStream) override;

	virtual bool HandleCollisionWithCar(PlayerCar* inCar) override;

	virtual void Update() override;

	bool IsActive() const { return is_collectable_; };
	void SetActive(bool can_collect);

	void ResetTimer();

protected:
	Mouse();

	void SetOldXPosition();

	virtual void Respawn();

private:
	bool is_collectable_;
	float time_until_respawn_;
	const float default_time_until_respawn_ = 2.f;

protected:
	float old_x_position_;
};

