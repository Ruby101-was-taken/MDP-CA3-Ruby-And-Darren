#include "RoboCatClientPCH.hpp"
// Darren Meidl - D000255479 - Entire class
CheckpointClient::CheckpointClient() 
{
	mSpriteComponent.reset(new SpriteComponent(this));
	mSpriteComponent->SetTexture(TextureManager::sInstance->GetTexture("checkpoint"));
	mPassed = false;
	mIsNext = false;
}

static void UpdateAllCheckpointVisualsForLocalPlayer(RoboCat* inCat)
{
	if (!inCat)
		return;

	// Derive total checkpoints from world objects (client-side)
	int totalCheckpoints = 0;
	const auto& gameObjects = World::sInstance->GetGameObjects();
	for (const auto& goPtr : gameObjects)
	{
		if (dynamic_cast<CheckpointClient*>(goPtr.get()))
		{
			++totalCheckpoints;
		}
	}

	if (totalCheckpoints == 0)
		return;

	int currentCpIndex = inCat->GetCurrentCheckpointIndex();
	int expectedIndex = (currentCpIndex >= 0) ? ((currentCpIndex + 1) % totalCheckpoints) : 0;

	for (const auto& goPtr : gameObjects)
	{
		CheckpointClient* cp = dynamic_cast<CheckpointClient*>(goPtr.get());
		if (!cp) continue;

		int idx = cp->GetIndex();

		// Passed if its index == current checkpoint (and current >= 0)
		if (currentCpIndex >= 0 && idx == currentCpIndex)
		{
			cp->SetPassed(true);
			cp->SetNext(false);
		}
		else
		{
			cp->SetPassed(false);
			// Mark next only for the expected index
			cp->SetNext(idx == expectedIndex);
		}
	}
}

bool CheckpointClient::HandleCollisionWithCat(RoboCat* inCat)
{
	// record lap before informing the base class (which calls inCat->OnCheckpointPassed)
	int oldLap = inCat ? inCat->GetCurrentLap() : -1;

	// Call base implementation to preserve game logic on client-side simulation
	bool result = Checkpoint::HandleCollisionWithCat(inCat);

	// Determine whether a lap advanced as a result of this pass
	int newLap = inCat ? inCat->GetCurrentLap() : -1;
	bool lapAdvanced = (inCat && newLap > oldLap);

	// Only update visual state for the local player's client.
	bool isLocalPlayer = (inCat && inCat->GetPlayerId() == NetworkManagerClient::sInstance->GetPlayerId());
	if (isLocalPlayer)
	{
		// Recompute visuals for all checkpoints based on the (validated) RoboCat current checkpoint.
		// This prevents invalid checkpoint collisions from turning other checkpoints green
		// because RoboCat::OnCheckpointPassed already enforces the expected index.
		UpdateAllCheckpointVisualsForLocalPlayer(inCat);

		// Update HUD for local player with current checkpoint / lap info
		int currentCpIndex = inCat->GetCurrentCheckpointIndex();
		int currentLap = inCat->GetCurrentLap();

		int totalCheckpoints = 0;
		const auto& gameObjects = World::sInstance->GetGameObjects();
		for (const auto& goPtr : gameObjects)
		{
			if (dynamic_cast<CheckpointClient*>(goPtr.get()))
			{
				++totalCheckpoints;
			}
		}

		int lapsToWin = inCat->GetLapsToWin();
		if (HUD::sInstance)
		{
			HUD::sInstance->SetPlayerRaceProgress(currentCpIndex, totalCheckpoints, currentLap, lapsToWin);
		}
	}

	// Keep original return value (usually false to skip physical response)
	return result;
}

void CheckpointClient::SetPassed(bool inPassed)
{
	if (mPassed == inPassed)
		return;

	mPassed = inPassed;

	// If passed, always use passed texture. If not passed, rely on mIsNext to choose
	if (mPassed)
	{
		mSpriteComponent->SetTexture(TextureManager::sInstance->GetTexture("checkpoint_passed"));
	}
	else
	{
		// choose next texture if flagged, otherwise default
		if (mIsNext)
			mSpriteComponent->SetTexture(TextureManager::sInstance->GetTexture("checkpoint_next"));
		else
			mSpriteComponent->SetTexture(TextureManager::sInstance->GetTexture("checkpoint"));
	}
}

void CheckpointClient::SetNext(bool inIsNext)
{
	if (mIsNext == inIsNext)
		return;

	mIsNext = inIsNext;

	// Only change texture when not already passed
	if (!mPassed)
	{
		if (mIsNext)
			mSpriteComponent->SetTexture(TextureManager::sInstance->GetTexture("checkpoint_next"));
		else
			mSpriteComponent->SetTexture(TextureManager::sInstance->GetTexture("checkpoint"));
	}
}