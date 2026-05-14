#include "RoboCatClientPCH.hpp"

std::unique_ptr< HUD >	HUD::sInstance;


HUD::HUD() :
	mScoreBoardOrigin(50.f, 60.f, 0.0f),
	mBandwidthOrigin(50.f, 10.f, 0.0f),
	mRoundTripTimeOrigin(580.f, 10.f, 0.0f),
	mScoreOffset(0.f, 50.f, 0.0f),
	mRaceInfoOrigin(900.f, 60.f, 0.0f),
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
	// Save current view (world view), switch to default (screen) view so HUD is drawn in screen coordinates
	sf::View previousView = WindowManager::sInstance->getView();
	WindowManager::sInstance->setView(WindowManager::sInstance->getDefaultView());

	
	RenderRaceOver();
	RenderBandWidth();
	RenderRoundTripTime();
	RenderScoreBoard();
	RenderRaceInfo();
	RenderHostStartPrompt();
	RenderLobbyWaitingScreen();

	// Restore world view for any further world rendering / display
	WindowManager::sInstance->setView(previousView);
}
// Darren Meidl - D00255479 - Render start prompt for host player when in lobby
void HUD::RenderHostStartPrompt() {
	if (NetworkManagerClient::sInstance && NetworkManagerClient::sInstance->IsLobbyOpen()) {
		// only the host (player 1) sees the start prompt
		if (NetworkManagerClient::sInstance->GetPlayerId() == 1) {
			Vector3 startOrigin(250.f, 350.f, 0.f);

			// Create full-screen black background (slightly transparent)
			sf::View defaultView = WindowManager::sInstance->getDefaultView();
			sf::Vector2f viewSize = defaultView.getSize();
			sf::RectangleShape background(viewSize);
			background.setPosition(0.f, 0.f);
			background.setFillColor(sf::Color(0, 0, 0, 255));
			WindowManager::sInstance->draw(background);

			// Create text
			sf::Text text;
			const string prompt = "Press 'S' to START RACE (Host Only)";
			text.setString(prompt);
			text.setFillColor(sf::Color(255, 255, 255, 255));
			text.setCharacterSize(50);
			text.setPosition(startOrigin.mX, startOrigin.mY);
			text.setFont(*FontManager::sInstance->GetFont("carlito"));

			WindowManager::sInstance->draw(text);
		}
	}
}

// New: Render a fullscreen black screen with centered message for non-host players waiting in lobby
void HUD::RenderLobbyWaitingScreen()
{
	if (!(NetworkManagerClient::sInstance && NetworkManagerClient::sInstance->IsLobbyOpen()))
		return;

	// only non-host players (host is player 1) should see this waiting screen
	if (NetworkManagerClient::sInstance->GetPlayerId() == 1)
		return;

	// Full-screen black background
	sf::View defaultView = WindowManager::sInstance->getDefaultView();
	sf::Vector2f viewSize = defaultView.getSize();
	sf::RectangleShape background(viewSize);
	background.setPosition(0.f, 0.f);
	background.setFillColor(sf::Color(0, 0, 0, 255));
	WindowManager::sInstance->draw(background);

	// Centered message
	sf::Text text;
	const string prompt = "You're in! Waiting on Host to Start.";
	text.setString(prompt);
	text.setFillColor(sf::Color(255, 255, 255, 255));
	text.setCharacterSize(50);
	text.setFont(*FontManager::sInstance->GetFont("carlito"));

	// center the text
	sf::FloatRect bounds = text.getLocalBounds();
	text.setOrigin(bounds.left + bounds.width / 2.f, bounds.top + bounds.height / 2.f);
	text.setPosition(viewSize.x / 2.f, viewSize.y / 2.f);

	WindowManager::sInstance->draw(text);
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

// Darren Meidl - D00255479 - Render race over
void HUD::RenderRaceOver()
{
	if (!ScoreBoardManager::sInstance)
		return;

	if (!ScoreBoardManager::sInstance->GetIsGameOver())
		return;

	const vector<uint32_t>& winners = ScoreBoardManager::sInstance->GetWinners();
	if (winners.empty())
		return;

	// Draw a semi-transparent fullscreen background to darken the scene
	sf::View defaultView = WindowManager::sInstance->getDefaultView();
	sf::Vector2f viewSize = defaultView.getSize();
	sf::RectangleShape background(viewSize);
	background.setPosition(0.f, 0.f);
	background.setFillColor(sf::Color(0, 0, 0, 200)); // semi-transparent black
	WindowManager::sInstance->draw(background);

	// Title: "Race Standings" centered at the top
	sf::Text title;
	title.setFont(*FontManager::sInstance->GetFont("carlito"));
	title.setString("Race Standings");
	title.setCharacterSize(70);
	title.setFillColor(sf::Color(255, 255, 255, 255));
	// center title
	sf::FloatRect tBounds = title.getLocalBounds();
	title.setOrigin(tBounds.left + tBounds.width / 2.f, tBounds.top + tBounds.height / 2.f);
	title.setPosition(viewSize.x / 2.f, 80.f);
	WindowManager::sInstance->draw(title);

	// List all finishers in order, center each line, highlight 1st in gold
	const float startY = 160.f;
	const float lineSpacing = 60.f;
	for (size_t idx = 0; idx < winners.size(); ++idx)
	{
		uint32_t pid = winners[idx];
		const auto* entry = ScoreBoardManager::sInstance->GetEntry(pid);
		string name = entry ? entry->GetPlayerName() : StringUtils::Sprintf("Player %u", pid);
		string line = StringUtils::Sprintf("%d. %s", static_cast<int>(idx + 1), name.c_str());

		sf::Text lineText;
		lineText.setFont(*FontManager::sInstance->GetFont("carlito"));
		lineText.setString(line);
		lineText.setCharacterSize(50);

		// Winner in gold color, others in white
		if (idx == 0)
		{
			lineText.setFillColor(sf::Color(212, 175, 55, 255)); // gold-ish
		}
		else
		{
			lineText.setFillColor(sf::Color(255, 255, 255, 255));
		}

		// center the text horizontally
		sf::FloatRect b = lineText.getLocalBounds();
		lineText.setOrigin(b.left + b.width / 2.f, b.top + b.height / 2.f);
		lineText.setPosition(viewSize.x / 2.f, startY + idx * lineSpacing);
		WindowManager::sInstance->draw(lineText);
	}

	// Optionally show up to a max number to avoid overflowing the screen.
	// The current loop shows all entries received in the winners vector.
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

