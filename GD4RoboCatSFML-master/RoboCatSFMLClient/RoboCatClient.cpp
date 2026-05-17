#include "RoboCatClientPCH.hpp"

PlayerCarClient::PlayerCarClient() :
	mTimeLocationBecameOutOfSync(0.f),
	mTimeVelocityBecameOutOfSync(0.f)
{
	mSpriteComponent.reset(new PlayerSpriteComponent(this));
	mSpriteComponent->SetTexture(TextureManager::sInstance->GetTexture("car")); // Darren Meidl - D00255479 - New car sprite
	sf::Color colour = ColourUtilities::GetUserColourFromFile();
	SetColor(Vector3(colour.r, colour.g, colour.b));

	HUD::sInstance->SetInRaceStatus(true);

	// this sucks but it works so it stays
	if(NetworkManagerClient::sInstance->GetPlayerId() != 1)
		SoundManager::sInstance->PlayMusic("../Assets/Sound/Music/Theme/Race.mp3");
}

void PlayerCarClient::HandleDying() {
	PlayerCar::HandleDying();

	//and if we're local, tell the hud so our health goes away!
	if (GetPlayerId() == NetworkManagerClient::sInstance->GetPlayerId())
	{
		HUD::sInstance->SetPlayerHealth(0);
	}
}


void PlayerCarClient::Update() {
	//is this the cat owned by us?
	if (GetPlayerId() == NetworkManagerClient::sInstance->GetPlayerId())
	{
		// If lobby is open and this client is not host, suppress client-side prediction
		if (NetworkManagerClient::sInstance->IsLobbyOpen() && NetworkManagerClient::sInstance->GetPlayerId() != 1)
		{
			// Clear pending move(s) so they don't accumulate while lobby is open
			InputManager::sInstance->GetMoveList().Clear();

			// Do not apply any input or simulate movement while waiting in the lobby
			return;
		}

		const Move* pendingMove = InputManager::sInstance->GetAndClearPendingMove();
		//in theory, only do this if we want to sample input this frame / if there's a new move ( since we have to keep in sync with server )
		if (pendingMove) //is it time to sample a new move...
		{
			float deltaTime = pendingMove->GetDeltaTime();

			//apply that input
			ProcessInput(deltaTime, pendingMove->GetInputState());

			//and simulate!
			SimulateMovement(deltaTime);
		}
	}
	else
	{
		SimulateMovement(Timing::sInstance.GetDeltaTime());

		if (RoboMath::Is2DVectorEqual(GetVelocity(), Vector3::Zero))
		{
			//we're in sync if our velocity is 0
			mTimeLocationBecameOutOfSync = 0.f;
		}
	}

	//Logging::ClearLog();
	//Logging::Log("Player Position", std::to_string(GetLocation().mX) + ", " + std::to_string(GetLocation().mY));
}

void PlayerCarClient::Read(InputMemoryBitStream& inInputStream)
{
	bool stateBit;

	uint32_t readState = 0;

	inInputStream.Read(stateBit);
	if (stateBit)
	{
		uint32_t playerId;
		inInputStream.Read(playerId);
		SetPlayerId(playerId);
		readState |= ECRS_PlayerId;
	}

	float oldRotation = GetRotation();
	Vector3 oldLocation = GetLocation();
	Vector3 oldVelocity = GetVelocity();

	float replicatedRotation;
	Vector3 replicatedLocation;
	Vector3 replicatedVelocity;

	inInputStream.Read(stateBit);
	if (stateBit)
	{
		inInputStream.Read(replicatedVelocity.mX);
		inInputStream.Read(replicatedVelocity.mY);

		SetVelocity(replicatedVelocity);

		inInputStream.Read(replicatedLocation.mX);
		inInputStream.Read(replicatedLocation.mY);

		SetLocation(replicatedLocation);

		inInputStream.Read(replicatedRotation);
		SetRotation(replicatedRotation);

		readState |= ECRS_Pose;
	}

	inInputStream.Read(stateBit);
	if (stateBit)
	{
		inInputStream.Read(stateBit);
		mThrustDir = stateBit ? 1.f : -1.f;
	}
	else
	{
		mThrustDir = 0.f;
	}

	inInputStream.Read(stateBit);
	if (stateBit)
	{
		Vector3 color;
		inInputStream.Read(color);
		if (GetPlayerId() != NetworkManagerClient::sInstance->GetPlayerId())
			SetColor(color);
		readState |= ECRS_Color;
	}

	inInputStream.Read(stateBit);
	if (stateBit)
	{
		mHealth = 0;
		inInputStream.Read(mHealth, 4);
		readState |= ECRS_Health;
	}

	if (GetPlayerId() == NetworkManagerClient::sInstance->GetPlayerId())
	{
		//did we get health? if so, tell the hud!
		if ((readState & ECRS_Health) != 0)
		{
			HUD::sInstance->SetPlayerHealth(mHealth);
		}

		DoClientSidePredictionAfterReplicationForLocalCat(readState);

		//if this is a create packet, don't interpolate
		if ((readState & ECRS_PlayerId) == 0)
		{
			InterpolateClientSidePrediction(oldRotation, oldLocation, oldVelocity, false);
		}
	}
	else
	{
		DoClientSidePredictionAfterReplicationForRemoteCat(readState);

		//will this smooth us out too? it'll interpolate us just 10% of the way there...
		if ((readState & ECRS_PlayerId) == 0)
		{
			InterpolateClientSidePrediction(oldRotation, oldLocation, oldVelocity, true);
		}

	}


	inInputStream.Read(stateBit);
	if (stateBit) {
		mMaxLinearSpeed = 0;
		inInputStream.Read(mMaxLinearSpeed);
		readState |= ECRS_Speed;
	}

	inInputStream.Read(stateBit);
	if (stateBit) {
		int total_checkpoints = 0;
		inInputStream.Read(total_checkpoints);
		readState |= ECRS_Checkpoints;
		SetTotalCheckpoints(total_checkpoints);
	}
}

void PlayerCarClient::DoClientSidePredictionAfterReplicationForLocalCat(uint32_t inReadState)
{
	// If lobby is open and this client is not host, skip replaying moves (no prediction)
	if (NetworkManagerClient::sInstance->IsLobbyOpen() && NetworkManagerClient::sInstance->GetPlayerId() != 1)
	{
		// clear move list to avoid replay/backlog when lobby closes
		InputManager::sInstance->GetMoveList().Clear();
		return;
	}

	if ((inReadState & ECRS_Pose) != 0)
	{
		//simulate pose only if we received new pose- might have just gotten thrustDir
		//in which case we don't need to replay moves because we haven't warped backwards

		//all processed moves have been removed, so all that are left are unprocessed moves
		//so we must apply them...
		const MoveList& moveList = InputManager::sInstance->GetMoveList();

		for (const Move& move : moveList)
		{
			float deltaTime = move.GetDeltaTime();
			ProcessInput(deltaTime, move.GetInputState());

			SimulateMovement(deltaTime);
		}
	}
}


void PlayerCarClient::InterpolateClientSidePrediction(float inOldRotation, const Vector3 & inOldLocation, const Vector3 & inOldVelocity, bool inIsForRemoteCat)
{
	if (inOldRotation != GetRotation() && !inIsForRemoteCat)
	{
		LOG("ERROR! Move replay ended with incorrect rotation!", 0);
	}

	float roundTripTime = NetworkManagerClient::sInstance->GetRoundTripTime();

	if (!RoboMath::Is2DVectorEqual(inOldLocation, GetLocation()))
	{
		//LOG( "ERROR! Move replay ended with incorrect location!", 0 );

		//have we been out of sync, or did we just become out of sync?
		float time = Timing::sInstance.GetFrameStartTime();
		if (mTimeLocationBecameOutOfSync == 0.f)
		{
			mTimeLocationBecameOutOfSync = time;
		}

		float durationOutOfSync = time - mTimeLocationBecameOutOfSync;
		if (durationOutOfSync < roundTripTime)
		{
			SetLocation(Lerp(inOldLocation, GetLocation(), inIsForRemoteCat ? (durationOutOfSync / roundTripTime) : 0.1f));
		}
	}
	else
	{
		//we're in sync
		mTimeLocationBecameOutOfSync = 0.f;
	}


	if (!RoboMath::Is2DVectorEqual(inOldVelocity, GetVelocity()))
	{
		//LOG( "ERROR! Move replay ended with incorrect velocity!", 0 );

		//have we been out of sync, or did we just become out of sync?
		float time = Timing::sInstance.GetFrameStartTime();
		if (mTimeVelocityBecameOutOfSync == 0.f)
		{
			mTimeVelocityBecameOutOfSync = time;
		}

		//now interpolate to the correct value...
		float durationOutOfSync = time - mTimeVelocityBecameOutOfSync;
		if (durationOutOfSync < roundTripTime)
		{
			SetVelocity(Lerp(inOldVelocity, GetVelocity(), inIsForRemoteCat ? (durationOutOfSync / roundTripTime) : 0.1f));
		}
		//otherwise, fine...

	}
	else
	{
		//we're in sync
		mTimeVelocityBecameOutOfSync = 0.f;
	}

}


//so what do we want to do here? need to do some kind of interpolation...

void PlayerCarClient::DoClientSidePredictionAfterReplicationForRemoteCat(uint32_t inReadState)
{
	if ((inReadState & ECRS_Pose) != 0)
	{

		//simulate movement for an additional RTT
		float rtt = NetworkManagerClient::sInstance->GetRoundTripTime();
		//LOG( "Other cat came in, simulating for an extra %f", rtt );

		//let's break into framerate sized chunks though so that we don't run through walls and do crazy things...
		float deltaTime = 1.f / 30.f;

		while (true)
		{
			if (rtt < deltaTime)
			{
				SimulateMovement(rtt);
				break;
			}
			else
			{
				SimulateMovement(deltaTime);
				rtt -= deltaTime;
			}
		}
	}
}

void PlayerCarClient::OnCompleteLap() {
	PlayerCar::OnCompleteLap();
	SoundManager::sInstance->Play("Lap");
	if(OnFinalLap() and GetPlayerId() == NetworkManagerClient::sInstance->GetPlayerId())
		SoundManager::sInstance->PlayMusic("../Assets/Sound/Music/Theme/Final.wav");
}

void PlayerCarClient::OnCompleteRace() {
	PlayerCar::OnCompleteRace();
	Logging::Log("PlayerCarClient::OnCompleteRace", "Player " + std::to_string(GetPlayerId()) + " has completed the race!");
}

