#pragma once
#include "GameObject.hpp"
// Darren Meidl - D00255479 - Entire class
class Checkpoint : public GameObject {
public:
	CLASS_IDENTIFICATION('CHKP', Checkpoint)

	Checkpoint() : GameObject(), mIndex(0) {}
	Checkpoint(int inIndex) : GameObject(), mIndex(inIndex) {}

	void SetIndex(int inIndex) { mIndex = inIndex; }
	int GetIndex() const { return mIndex; }

	// When a cat collides with a checkpoint, notify the cat.
	// Return false to skip the physics collision response (no bounce).
	virtual bool HandleCollisionWithCat(RoboCat* inCat) override;

private:
	int mIndex;
};