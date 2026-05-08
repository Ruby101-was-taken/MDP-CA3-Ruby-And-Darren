#include "RoboCatClientPCH.hpp"

std::unique_ptr< HUD >	HUD::sInstance;


HUD::HUD() :
	mScoreBoardOrigin(50.f, 60.f, 0.0f),
	mBandwidthOrigin(50.f, 10.f, 0.0f),
	mRoundTripTimeOrigin(580.f, 10.f, 0.0f),
	mScoreOffset(0.f, 50.f, 0.0f),
	mHealthOffset(1000, 10.f, 0.0f),
	mRaceInfoOrigin(50.f, 110.f, 0.0f),
	mHealth(0),
	mPlayerCurrentCheckpointIndex(-1),
	mPlayerTotalCheckpoints(0),
	mPlayerCurrentLap(0),
	mPlayerLapsToWin(0)
{
}


void HUD::StaticInit()
{
	sInstance.reset(new HUD());
}

void HUD::Render()
{
	RenderGameOver();
	RenderBandWidth();
	RenderRoundTripTime();
	RenderScoreBoard();
	RenderHealth();
	RenderRaceInfo();
}

void HUD::RenderHealth()
{
	if (mHealth > 0)
	{
		string healthString = StringUtils::Sprintf("Health %d", mHealth);
		RenderText(healthString, mHealthOffset, Colors::Red);
	}
}

void HUD::RenderBandWidth()
{
	string bandwidth = StringUtils::Sprintf("In %d  Bps, Out %d Bps",
		static_cast<int>(NetworkManagerClient::sInstance->GetBytesReceivedPerSecond().GetValue()),
		static_cast<int>(NetworkManagerClient::sInstance->GetBytesSentPerSecond().GetValue()));
	RenderText(bandwidth, mBandwidthOrigin, Colors::White);
}

void HUD::RenderRoundTripTime()
{
	float rttMS = NetworkManagerClient::sInstance->GetAvgRoundTripTime().GetValue() * 1000.f;

	string roundTripTime = StringUtils::Sprintf("RTT %d ms", (int)rttMS);
	RenderText(roundTripTime, mRoundTripTimeOrigin, Colors::White);
}

void HUD::RenderScoreBoard()
{
	const vector< ScoreBoardManager::Entry >& entries = ScoreBoardManager::sInstance->GetEntries();
	Vector3 offset = mScoreBoardOrigin;

	for (const auto& entry : entries)
	{
		RenderText(entry.GetFormattedNameScore(), offset, entry.GetColor());
		offset.mX += mScoreOffset.mX;
		offset.mY += mScoreOffset.mY;
	}

}

// Darren Meidl - D00255479 - Render game over and winners if game is over
void HUD::RenderGameOver()
{
	if (!ScoreBoardManager::sInstance)
		return;

	if (!ScoreBoardManager::sInstance->GetIsGameOver())
		return;

	const vector<uint32_t>& winners = ScoreBoardManager::sInstance->GetWinners();
	if (winners.empty())
		return;

	// show Winner and up to top 3
	Vector3 origin(400.f, 30.f, 0.f); // top-center-ish; tweak as needed
	int idx = 0;
	for (uint32_t pid : winners)
	{
		const auto* entry = ScoreBoardManager::sInstance->GetEntry(pid);
		string text;
		if (idx == 0)
		{
			string name = entry ? entry->GetPlayerName() : StringUtils::Sprintf("Player %u", pid);
			text = StringUtils::Sprintf("Winner: %s", name.c_str());
			RenderText(text, origin, Colors::White);
		}
		else
		{
			string name = entry ? entry->GetPlayerName() : StringUtils::Sprintf("Player %u", pid);
			text = StringUtils::Sprintf("%d: %s", idx + 1, name.c_str());
			Vector3 subOrigin = origin;
			subOrigin.mY += 40.f * idx;
			RenderText(text, subOrigin, Colors::White);
		}
		++idx;
		// only show up to top 3
		if (idx >= 3) break;
	}
}

// Darren Meidl - D00255479 - Update local player's checkpoint and lap progress for HUD display
void HUD::SetPlayerRaceProgress(int inCurrentCheckpointIndex, int inTotalCheckpoints, int inCurrentLap, int inLapsToWin)
{
	mPlayerCurrentCheckpointIndex = inCurrentCheckpointIndex;
	mPlayerTotalCheckpoints = inTotalCheckpoints;
	mPlayerCurrentLap = inCurrentLap;
	mPlayerLapsToWin = inLapsToWin;
}
// Darren Meidl - D00255479 - Render checkpoint and lap info for local player
void HUD::RenderRaceInfo()
{
	// Checkpoint index may be -1 before any are passed; display as 0 in that case
	int displayCp = (mPlayerCurrentCheckpointIndex >= 0) ? (mPlayerCurrentCheckpointIndex + 1) : 0;
	int totalCp = mPlayerTotalCheckpoints;

	string cpString = StringUtils::Sprintf("Checkpoint %d / %d", displayCp, totalCp);
	RenderText(cpString, mRaceInfoOrigin, Colors::White);

	// Laps (display as 1-based)
	int displayLap = mPlayerCurrentLap + 1;
	int lapGoal = mPlayerLapsToWin > 0 ? mPlayerLapsToWin : 0;
	string lapString = StringUtils::Sprintf("Lap %d / %d", displayLap, lapGoal);
	Vector3 lapOrigin = mRaceInfoOrigin;
	lapOrigin.mY += 40.f;
	RenderText(lapString, lapOrigin, Colors::White);
}

void HUD::RenderText(const string& inStr, const Vector3& origin, const Vector3& inColor)
{
	sf::Text text;
	text.setString(inStr);
	text.setFillColor(sf::Color(inColor.mX, inColor.mY, inColor.mZ, 255));
	text.setCharacterSize(50);
	text.setPosition(origin.mX, origin.mY);
	text.setFont(*FontManager::sInstance->GetFont("carlito"));
	WindowManager::sInstance->draw(text);
}

