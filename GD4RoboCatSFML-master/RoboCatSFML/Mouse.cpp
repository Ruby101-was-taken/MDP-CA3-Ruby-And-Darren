#include "RoboCatPCH.hpp"
#include <fstream>

Mouse::Mouse() : 
	is_collectable_(true),
	time_until_respawn_(0.f)
{
	SetScale(GetScale() * 0.5f);
	SetCollisionRadius(40.f);
}

// Ruby White - D00255322
void Mouse::SetOldXPosition() {
	old_x_position_ = GetLocation().mX;
}
void Mouse::Respawn() {

}


bool Mouse::HandleCollisionWithCar(PlayerCar* inCar)
{
	if (!is_collectable_) {
		return false;
	}

	(void)inCar;
	return false;
}

void Mouse::SetActive(bool can_collect) {
	is_collectable_ = can_collect;
}

void Mouse::ResetTimer() {
	time_until_respawn_ = default_time_until_respawn_;
}

void Mouse::Update() {
	GameObject::Update();

	if (!is_collectable_) {
		time_until_respawn_ -= Timing::sInstance.GetDeltaTime();

		//respawn
		if (time_until_respawn_ <= 0.f) {
			Respawn();
			is_collectable_ = true;
		}
	}
}


uint32_t Mouse::Write(OutputMemoryBitStream& inOutputStream, uint32_t inDirtyState) const
{
	uint32_t writtenState = 0;

	if (inDirtyState & EMRS_Active) {
		inOutputStream.Write(true);
		inOutputStream.Write(is_collectable_);
;
		writtenState |= EMRS_Active;
	}
	else {
		inOutputStream.Write(false);
	}


	if (inDirtyState & EMRS_Pose)
	{
		inOutputStream.Write((bool)true);

		Vector3 location = GetLocation();
		inOutputStream.Write(location.mX);
		inOutputStream.Write(location.mY);

		inOutputStream.Write(GetRotation());

		writtenState |= EMRS_Pose;
	}
	else
	{
		inOutputStream.Write((bool)false);
	}



	return writtenState;
}

void Mouse::Read(InputMemoryBitStream& inInputStream)
{
	bool stateBit;

	inInputStream.Read(stateBit);
	if (stateBit) {
		bool can_collect;
		inInputStream.Read(can_collect);

		SetActive(can_collect);

	}

	inInputStream.Read(stateBit);
	if (stateBit)
	{
		Vector3 location;
		inInputStream.Read(location.mX);
		inInputStream.Read(location.mY);
		SetLocation(location);

		float rotation;
		inInputStream.Read(rotation);
		SetRotation(rotation);
		std::ofstream log("net_debug.txt", std::ios::app);
		log << "Position: " << is_collectable_ << "\n";
	}


}


