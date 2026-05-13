#include "RoboCatServerPCH.hpp"

MouseServer::MouseServer()
{
}

void MouseServer::HandleDying()
{
	NetworkManagerServer::sInstance->UnregisterGameObject(this);
}

void MouseServer::Respawn() {
    Logging::Log("MouseServer", "Respawn Method: " + std::to_string(old_x_position_));
    SetLocation(Vector3(old_x_position_, GetLocation().mY, GetLocation().mZ));
    NetworkManagerServer::sInstance->SetStateDirty(
        GetNetworkId(),
        EMRS_Pose
    );
    Logging::Log("MouseServer", "Respawn Method: " + std::to_string(GetLocation().mX));
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
    SetLocation(Vector3(100000, GetLocation().mY, GetLocation().mZ));
    NetworkManagerServer::sInstance->SetStateDirty(
        GetNetworkId(),
        EMRS_Pose
    );

    return false;
}