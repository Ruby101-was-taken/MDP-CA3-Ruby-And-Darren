#include "RoboCatPCH.hpp"

// Darren Meidl - D00255479 - Entire class
bool Checkpoint::HandleCollisionWithCat(RoboCat* inCat) {
	if (inCat) {
		inCat->OnCheckpointPassed(this);
	}
	// prevent physical collision response (we just want notification)
	return false;
}