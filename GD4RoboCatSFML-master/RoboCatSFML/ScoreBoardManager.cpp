#include "RoboCatPCH.hpp"

std::unique_ptr< ScoreBoardManager >	ScoreBoardManager::sInstance;


void ScoreBoardManager::StaticInit()
{
	sInstance.reset(new ScoreBoardManager());
}

ScoreBoardManager::ScoreBoardManager()
{
	mDefaultColors.push_back(Colors::LightYellow);
	mDefaultColors.push_back(Colors::LightBlue);
	mDefaultColors.push_back(Colors::LightPink);
	mDefaultColors.push_back(Colors::LightGreen);
}

ScoreBoardManager::Entry::Entry(uint32_t inPlayerId, const string& inPlayerName, const Vector3& inColor) :
	mPlayerId(inPlayerId),
	mPlayerName(inPlayerName),
	mColor(inColor)
{
	SetScore(0);
}

void ScoreBoardManager::Entry::SetScore(int32_t inScore)
{
	mScore = inScore;

	char	buffer[256];
	snprintf(buffer, 256, "%s %i", mPlayerName.c_str(), mScore);
	mFormattedNameScore = string(buffer);

}


ScoreBoardManager::Entry* ScoreBoardManager::GetEntry(uint32_t inPlayerId)
{
	for (Entry& entry : mEntries)
	{
		if (entry.GetPlayerId() == inPlayerId)
		{
			return &entry;
		}
	}

	return nullptr;
}

bool ScoreBoardManager::RemoveEntry(uint32_t inPlayerId)
{
	for (auto eIt = mEntries.begin(), endIt = mEntries.end(); eIt != endIt; ++eIt)
	{
		if ((*eIt).GetPlayerId() == inPlayerId)
		{
			mEntries.erase(eIt);
			return true;
		}
	}

	return false;
}

void ScoreBoardManager::AddEntry(uint32_t inPlayerId, const string& inPlayerName)
{
	//if this player id exists already, remove it first- it would be crazy to have two of the same id
	RemoveEntry(inPlayerId);

	mEntries.emplace_back(inPlayerId, inPlayerName, mDefaultColors[inPlayerId % mDefaultColors.size()]);
}

void ScoreBoardManager::IncScore(uint32_t inPlayerId, int inAmount)
{
	Entry* entry = GetEntry(inPlayerId);
	if (entry) {
		 if(entry->GetScore() < 20) entry->SetScore(entry->GetScore() + inAmount);
	}
}


// Darren Meidl - D00255479 - Determine race winners
void ScoreBoardManager::SetRaceWinners(int inTopN)
{
	mWinners.clear();
	mGameOver = false;

	if (inTopN <= 0)
		return;

	// gather (progress, playerId)
	std::vector<std::pair<int, uint32_t>> progressList;

	const auto& gameObjects = World::sInstance->GetGameObjects();
	for (const auto& goPtr : gameObjects)
	{
		PlayerCar* cat = goPtr->GetAsCar();
		if (cat)
		{
			int lap = cat->GetCurrentLap();
			int cpIndex = cat->GetCurrentCheckpointIndex();
			if (cpIndex < 0) cpIndex = 0;
			int progress = lap * 10000 + cpIndex;
			progressList.emplace_back(progress, cat->GetPlayerId());
		}
	}

	// sort descending by progress
	std::sort(progressList.begin(), progressList.end(), [](const auto& a, const auto& b) {
		return a.first > b.first;
	});

	int count = std::min(static_cast<int>(progressList.size()), inTopN);
	for (int i = 0; i < count; ++i)
	{
		mWinners.push_back(progressList[i].second);
	}

	if (!mWinners.empty())
	{
		mGameOver = true;
	}
}


bool ScoreBoardManager::Write(OutputMemoryBitStream& inOutputStream) const
{
	int entryCount = mEntries.size();

	//we don't know our player names, so it's hard to check for remaining space in the packet...
	//not really a concern now though
	inOutputStream.Write(entryCount);
	for (const Entry& entry : mEntries)
	{
		entry.Write(inOutputStream);
	}
	// Darren Meidl - D00255479 - Write game-over flag and winners
	inOutputStream.Write(mGameOver);
	if (mGameOver)
	{
		int winnerCount = static_cast<int>(mWinners.size());
		inOutputStream.Write(winnerCount);
		for (uint32_t pid : mWinners)
			inOutputStream.Write(pid);
	}

	return true;
}



bool ScoreBoardManager::Read(InputMemoryBitStream& inInputStream)
{
	int entryCount;
	inInputStream.Read(entryCount);
	//just replace everything that's here, it don't matter...
	mEntries.resize(entryCount);
	for (Entry& entry : mEntries)
	{
		entry.Read(inInputStream);
	}
	// Darren Meidl - D00255479 - Read game-over state and winners
	bool gameOver = false;
	inInputStream.Read(gameOver);
	mGameOver = gameOver;
	mWinners.clear();
	if (mGameOver)
	{
		int winnerCount = 0;
		inInputStream.Read(winnerCount);
		for (int i = 0; i < winnerCount; ++i)
		{
			uint32_t pid = 0;
			inInputStream.Read(pid);
			mWinners.push_back(pid);
		}
	}

	return true;
}


bool ScoreBoardManager::Entry::Write(OutputMemoryBitStream& inOutputStream) const
{
	bool didSucceed = true;

	inOutputStream.Write(mColor);
	inOutputStream.Write(mPlayerId);
	inOutputStream.Write(mPlayerName);
	inOutputStream.Write(mScore);

	return didSucceed;
}

bool ScoreBoardManager::Entry::Read(InputMemoryBitStream& inInputStream)
{
	bool didSucceed = true;

	inInputStream.Read(mColor);
	inInputStream.Read(mPlayerId);

	inInputStream.Read(mPlayerName);

	int score;
	inInputStream.Read(score);
	if (didSucceed)
	{
		SetScore(score);
	}


	return didSucceed;
}




