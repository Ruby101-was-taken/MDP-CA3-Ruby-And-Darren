#include "RoboCatServerPCH.hpp"

MouseServer::MouseServer()
{
}

void MouseServer::HandleDying()
{
	NetworkManagerServer::sInstance->UnregisterGameObject(this);
}

bool MouseServer::HandleCollisionWithCar(PlayerCar* inCar)
{
    if (!IsActive()) {
        return false;
    }

    SetActive(false);
    ResetTimer();

    ScoreBoardManager::sInstance->IncScore(inCar->GetPlayerId(), 1);

    return false;
}