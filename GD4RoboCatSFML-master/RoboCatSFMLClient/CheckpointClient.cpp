#include "RoboCatClientPCH.hpp"

CheckpointClient::CheckpointClient() 
{
	mSpriteComponent.reset(new SpriteComponent(this));
	mSpriteComponent->SetTexture(TextureManager::sInstance->GetTexture("checkpoint"));
	mPassed = false;
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
		if (lapAdvanced)
		{
			// Reset all checkpoint visuals on this client for the new lap
			const auto& gameObjects = World::sInstance->GetGameObjects();
			for (const auto& goPtr : gameObjects)
			{
				CheckpointClient* cp = dynamic_cast<CheckpointClient*>(goPtr.get());
				if (cp)
				{
					cp->SetPassed(false);
				}
			}
		}
		else
		{
			// Normal pass: mark this checkpoint as passed (only locally)
			SetPassed(true);
		}

		// Update HUD for local player with current checkpoint / lap info
		int currentCpIndex = inCat->GetCurrentCheckpointIndex();
		int currentLap = inCat->GetCurrentLap();

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
	if (mPassed)
	{
		mSpriteComponent->SetTexture(TextureManager::sInstance->GetTexture("checkpoint_passed"));
	}
	else
	{
		mSpriteComponent->SetTexture(TextureManager::sInstance->GetTexture("checkpoint"));
	}
}