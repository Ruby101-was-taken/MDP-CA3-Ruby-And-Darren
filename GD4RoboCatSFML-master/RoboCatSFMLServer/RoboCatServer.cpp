#include "RoboCatServerPCH.hpp"

void PlayerCarServer::IncreaseTopSpeed() {
	PlayerCar::IncreaseTopSpeed();
	NetworkManagerServer::sInstance->SetStateDirty(
		GetNetworkId(),
		ECRS_Speed
	);
}

PlayerCarServer::PlayerCarServer() :
	mCarControlType(ESCT_Human),
	mTimeOfNextShot(0.f),
	mTimeBetweenShots(0.2f)
{}

void PlayerCarServer::HandleDying()
{
	NetworkManagerServer::sInstance->UnregisterGameObject(this);
}

void PlayerCarServer::Update()
{
	PlayerCar::Update();

	Vector3 oldLocation = GetLocation();
	Vector3 oldVelocity = GetVelocity();
	float oldRotation = GetRotation();

	//are you controlled by a player?
	//if so, is there a move we haven't processed yet?
	if (mCarControlType == ESCT_Human)
	{
		ClientProxyPtr client = NetworkManagerServer::sInstance->GetClientProxy(GetPlayerId());
		if (client)
		{
			MoveList& moveList = client->GetUnprocessedMoveList();
			for (const Move& unprocessedMove : moveList)
			{
				const InputState& currentState = unprocessedMove.GetInputState();

				float deltaTime = unprocessedMove.GetDeltaTime();

				ProcessInput(deltaTime, currentState);
				SimulateMovement(deltaTime);

				//LOG( "Server Move Time: %3.4f deltaTime: %3.4f left rot at %3.4f", unprocessedMove.GetTimestamp(), deltaTime, GetRotation() );

			}

			moveList.Clear();
		}
	}
	else
	{
		//do some AI stuff
		SimulateMovement(Timing::sInstance.GetDeltaTime());
	}


	//HandleShooting();

	//if (!RoboMath::Is2DVectorEqual(oldLocation, GetLocation()) ||
	//	!RoboMath::Is2DVectorEqual(oldVelocity, GetVelocity()) ||
	//	oldRotation != GetRotation())
	//{
	//	NetworkManagerServer::sInstance->SetStateDirty(GetNetworkId(), ECRS_Pose);
	//}
}

void PlayerCarServer::HandleShooting()
{
	float time = Timing::sInstance.GetFrameStartTime();
	if (mIsShooting && Timing::sInstance.GetFrameStartTime() > mTimeOfNextShot)
	{
		//not exact, but okay
		mTimeOfNextShot = time + mTimeBetweenShots;

		//fire!
		YarnPtr yarn = std::static_pointer_cast<Yarn>(GameObjectRegistry::sInstance->CreateGameObject('YARN'));
		yarn->InitFromShooter(this);
	}
}

void PlayerCarServer::TakeDamage(int inDamagingPlayerId)
{
	mHealth--;
	if (mHealth <= 0.f)
	{
		//score one for damaging player...
		ScoreBoardManager::sInstance->IncScore(inDamagingPlayerId, 1);

		//and you want to die
		SetDoesWantToDie(true);

		//tell the client proxy to make you a new car
		ClientProxyPtr clientProxy = NetworkManagerServer::sInstance->GetClientProxy(GetPlayerId());
		if (clientProxy)
		{
			clientProxy->HandleCarDied();
		}
	}

	//tell the world our health dropped
	NetworkManagerServer::sInstance->SetStateDirty(GetNetworkId(), ECRS_Health);
}

