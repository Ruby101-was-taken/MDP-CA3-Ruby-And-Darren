#include "RoboCatServerPCH.hpp"
// Darren Meidl - D000255479 - Entire class
CheckpointServer::CheckpointServer() {
}

void CheckpointServer::HandleDying() {
	NetworkManagerServer::sInstance->UnregisterGameObject(this);
}
bool CheckpointServer::HandleCollisionWithCar(PlayerCar* inCar) {
	// Notify the server-side PlayerCar so it can update laps/checkpoints
	if (inCar) {
		inCar->OnCheckpointPassed(static_cast<Checkpoint*>(this));

		// If the car finished the race, mark game over and compute winners
		if (inCar->IsRaceFinished()) {
			LOG("Player %d finished race. Setting game over and computing winners.", inCar->GetPlayerId());
			ScoreBoardManager::sInstance->SetRaceWinners(3); // compute top 3 winners (by progress) and mark game over
			NetworkManagerServer::sInstance->UpdateAllClients(); // immediately push a state update to all clients so they know about game over
		}
	}
	return false; // Prevent physical collision response
}