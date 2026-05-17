#include "RoboCatServerPCH.hpp"

StarServer::StarServer()
{
}

void StarServer::HandleDying()
{
	NetworkManagerServer::sInstance->UnregisterGameObject(this);
}

void StarServer::Respawn() {
    SetLocation(Vector3(old_x_position_, GetLocation().mY, GetLocation().mZ));
    NetworkManagerServer::sInstance->SetStateDirty(
        GetNetworkId(),
        EMRS_Pose
    );
}

bool StarServer::HandleCollisionWithCar(PlayerCar* inCar) {
    if (!IsActive()) {
        return false;
    }

    SetActive(false);
    ResetTimer();

    ScoreBoardManager::sInstance->IncScore(inCar->GetPlayerId(), 1);

    SetOldXPosition();
    SetLocation(Vector3(100000, GetLocation().mY, GetLocation().mZ));
    NetworkManagerServer::sInstance->SetStateDirty(
        GetNetworkId(),
        EMRS_Pose
    );

    inCar->IncreaseTopSpeed();

    return false;
}