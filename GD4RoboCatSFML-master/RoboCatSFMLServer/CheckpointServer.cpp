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

        // If the car finished the race, notify RaceManager
        if (inCar->IsRaceFinished()) {
            LOG("Player %d finished race. Notifying RaceManager.", inCar->GetPlayerId());
            if (RaceManager::sInstance) {
                RaceManager::sInstance->OnPlayerFinished(inCar);
            }
        }
    }
    return false; // Prevent physical collision response
}