#include "RoboCatServerPCH.hpp"

MouseServer::MouseServer()
{
}

void MouseServer::HandleDying()
{
	NetworkManagerServer::sInstance->UnregisterGameObject(this);
}

void MouseServer::Respawn() {
    SetLocation(Vector3(old_x_position_, 0, 0));
    NetworkManagerServer::sInstance->SetStateDirty(
        GetNetworkId(),
        EMRS_Active
    );
}

bool MouseServer::HandleCollisionWithCar(PlayerCar* inCar)
{
    if (!IsActive()) {
        return false;
    }

    SetActive(false);
    ResetTimer();

    ScoreBoardManager::sInstance->IncScore(inCar->GetPlayerId(), 1);

    SetOldXPosition();
    SetLocation(Vector3(100000, 0, 0));
    NetworkManagerServer::sInstance->SetStateDirty(
        GetNetworkId(),
        EMRS_Pose
    );

    return false;
}