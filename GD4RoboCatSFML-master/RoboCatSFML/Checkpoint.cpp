#include "RoboCatPCH.hpp"

// Darren Meidl - D00255479 - Entire class
bool Checkpoint::HandleCollisionWithCar(PlayerCar* inCat) {
	if (inCat) {
		inCat->OnCheckpointPassed(this);
	}
	// prevent physical collision response (we just want notification)
	return false;
}

uint32_t Checkpoint::Write(OutputMemoryBitStream& inOutputStream, uint32_t inDirtyState) const
{
	uint32_t writtenState = 0;

	// Pose (location)
	if (inDirtyState & ECRS_Pose)
	{
		inOutputStream.Write((bool)true);
		const Vector3& loc = GetLocation();
		inOutputStream.Write(loc.mX);
		inOutputStream.Write(loc.mY);

		// Ruby White - D00255322
		inOutputStream.Write(GetRotation());

		inOutputStream.Write(GetScale());

		// rotation/scale aren't meaningful for checkpoint visuals in this project, skip unless needed <- rotation and scale were needed, thanks Darren
		writtenState |= ECRS_Pose;
	}
	else
	{
		inOutputStream.Write((bool)false);
	}

	// Index
	if (inDirtyState & ECRS_Index)
	{
		inOutputStream.Write((bool)true);
		inOutputStream.Write(mIndex, 8);
		writtenState |= ECRS_Index;
	}
	else
	{
		inOutputStream.Write((bool)false);
	}

	return writtenState;
}

void Checkpoint::Read(InputMemoryBitStream& inInputStream)
{
	bool isPoseDirty = false;
	inInputStream.Read(isPoseDirty);
	if (isPoseDirty)
	{
		float x = 0.f, y = 0.f;
		inInputStream.Read(x);
		inInputStream.Read(y);
		SetLocation(Vector3(x, y, 0.f));
		// Ruby White - D00255322
		float rot = 0.f;
		inInputStream.Read(rot);
		SetRotation(rot);
		float scale = 0.f;
		inInputStream.Read(scale);
		SetScale(scale);
	}

	bool isIndexDirty = false;
	inInputStream.Read(isIndexDirty);
	if (isIndexDirty)
	{
		int idx = 0;
		inInputStream.Read(idx, 8);
		mIndex = idx;
	}
}