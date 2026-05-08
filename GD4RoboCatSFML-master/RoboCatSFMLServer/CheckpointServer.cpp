#include "RoboCatServerPCH.hpp"
// Darren Meidl - D000255479 - Entire class
CheckpointServer::CheckpointServer() {
}

void CheckpointServer::HandleDying() {
	NetworkManagerServer::sInstance->UnregisterGameObject(this);
}
bool CheckpointServer::HandleCollisionWithCat(RoboCat* inCat) {
	// Notify the server-side cat so it can update laps/checkpoints.
	if (inCat) {
		inCat->OnCheckpointPassed(static_cast<Checkpoint*>(this));

		// If the cat finished the race, disconnect the client so you can test lap/checkpoint progression.
		if (inCat->IsRaceFinished()) {
			ClientProxyPtr client = NetworkManagerServer::sInstance->GetClientProxy(inCat->GetPlayerId());
			if (client) {
				LOG("Player %d finished race. Forcing server-side disconnect for test.", inCat->GetPlayerId());
				// TODO: Find cleaner way to do this & move this function back to private
				NetworkManagerServer::sInstance->HandleClientDisconnected(client); // Remove the client on the server
			}
		}
	}
	// Prevent physical collision response (we only want the notification).
	return false;
}