#include "RoboCatServerPCH.hpp"

CheckpointServer::CheckpointServer() {
}

void CheckpointServer::HandleDying() {
	NetworkManagerServer::sInstance->UnregisterGameObject(this);
}