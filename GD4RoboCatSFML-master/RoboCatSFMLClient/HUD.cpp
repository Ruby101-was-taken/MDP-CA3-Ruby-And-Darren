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
	mPlayerLapsToWin(0),
	mPlayerHasFinished(false)
{
}
void HUD::SetInRaceStatus(bool in_race) {
	in_race_ = in_race;
	Logging::Log("HUD", std::to_string(in_race_));
}

bool HUD::IsInRace() {
	return in_race_;
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

	
	
	RenderBandWidth();
	RenderRoundTripTime();
	//RenderScoreBoard();
	RenderHUD();
	RenderRaceInProgressJoinScreen();
	RenderRaceFinishedWaitingScreen();
	RenderRaceOver();
	RenderLobbyWaitingScreen();
	RenderClientWaitingScreen();

	// Restore world view for any further world rendering / display
	WindowManager::sInstance->setView(previousView);
}
// Darren Meidl - D00255479 - Render start prompt for host player when in lobby
void HUD::RenderLobbyWaitingScreen() {
	if (NetworkManagerClient::sInstance && NetworkManagerClient::sInstance->IsLobbyOpen()) {
		string prompt;
		if (NetworkManagerClient::sInstance->GetPlayerId() == 1) {
			prompt = "Press 'ENTER' to START RACE.";
		}
		else {
			prompt = "Waiting on Host to Start.";
		}
			
		
		
		// Create full-screen black background (slightly transparent)
		sf::View defaultView = WindowManager::sInstance->getDefaultView();
		sf::Vector2f viewSize = defaultView.getSize();
		sf::RectangleShape background(viewSize);
		background.setPosition(0.f, 0.f);
		background.setFillColor(sf::Color(0, 0, 0, 225));
		WindowManager::sInstance->draw(background);

		Vector3 startOrigin(50.f, 350.f, 0.f);
		sf::Text text;
			
		text.setString(prompt);
		text.setFillColor(sf::Color(255, 255, 255, 255));
		text.setCharacterSize(50);
		text.setPosition(startOrigin.mX, startOrigin.mY);
		text.setFont(*FontManager::sInstance->GetFont("carlito"));

		WindowManager::sInstance->draw(text);

		// Draw a neat list of all players in their respective colors (to the right / center area)
		if (ScoreBoardManager::sInstance)
		{
			const vector< ScoreBoardManager::Entry >& entries = ScoreBoardManager::sInstance->GetEntries();

			// Title for player list
			sf::Text listTitle;
			listTitle.setFont(*FontManager::sInstance->GetFont("carlito"));
			listTitle.setString("Players:");
			listTitle.setCharacterSize(48);
			listTitle.setFillColor(sf::Color(255, 255, 255, 255));

			// Position the player list on the right-center area
			const float listX = viewSize.x * 0.65f;
			const float listStartY = 120.f;
			const float lineSpacing = 48.f;

			listTitle.setPosition(listX - 70.f, listStartY - lineSpacing);
			WindowManager::sInstance->draw(listTitle);

			// Attempt to fetch the car texture once
			auto carTex = TextureManager::sInstance->GetTexture("car");

			for (size_t i = 0; i < entries.size(); ++i)
			{
				const auto& e = entries[i];

				// Text for player name
				sf::Text lineText;
				lineText.setFont(*FontManager::sInstance->GetFont("carlito"));
				lineText.setString(e.GetPlayerName());

				// Use a character size consistent with list spacing / title
				const unsigned int charSize = 48;
				lineText.setCharacterSize(charSize);

				Vector3 col = e.GetColor(); // Use the entry color
				// clamp/conversion to uint8_t
				auto toU8 = [](float v) -> uint8_t {
					int iv = static_cast<int>(std::round(v));
					if (iv < 0) iv = 0;
					if (iv > 255) iv = 255;
					return static_cast<uint8_t>(iv);
				};
				lineText.setFillColor(sf::Color(toU8(col.mX), toU8(col.mY), toU8(col.mZ), 255));

				// Calculate vertical center for this line based on text
				sf::FloatRect textBounds = lineText.getLocalBounds();
				float lineTopY = listStartY + static_cast<float>(i) * lineSpacing;
				float textCenterY = lineTopY - textBounds.top + textBounds.height / 2.f;

				// If we have a car texture: draw a tinted, west-facing sprite to the left of the name
				float spriteWidth = 0.f;
				if (carTex)
				{
					sf::Sprite carSprite;
					carSprite.setTexture(*carTex);

					sf::Vector2u tSize = carTex->getSize();
					if (tSize.y > 0)
					{
						float targetHeight = static_cast<float>(charSize);
						float scale = targetHeight / static_cast<float>(tSize.y);
						carSprite.setScale(0.08f, 0.08f);

						spriteWidth = static_cast<float>(tSize.x) * scale;
					}

					carSprite.setOrigin(static_cast<float>(tSize.x) * 0.5f, static_cast<float>(tSize.y) * 0.5f); // Set origin to center so rotation is around sprite center
					carSprite.setRotation(270.f); // rotate so texture faces left
					carSprite.setColor(sf::Color(toU8(col.mX), toU8(col.mY), toU8(col.mZ), 255)); // Tint using player colour

					// Position sprite: left-aligned at listX, vertically centered on the same line as the text
					float spritePosX = listX + spriteWidth - 50.f; // origin is centered
					float spritePosY = textCenterY + 25.f;
					carSprite.setPosition(spritePosX, spritePosY);

					WindowManager::sInstance->draw(carSprite);
				}

				// Position text to the right of the sprite with a small gap
				const float gap = 12.f;
				float textPosX = listX + spriteWidth + gap;
				lineText.setPosition(textPosX, lineTopY);
				WindowManager::sInstance->draw(lineText);
			}
		}
		
	}
}

// Darren Meidl - D00255479
void HUD::RenderRaceInProgressJoinScreen()
{
	if (!NetworkManagerClient::sInstance)
		return;

	if (NetworkManagerClient::sInstance->IsLobbyOpen())
		return;

	if (NetworkManagerClient::sInstance->GetPlayerId() > 0)
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
	const string prompt = "Race has started. Please wait until race is complete to join.";
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
// Darren Meidl - D00255479 - Render waiting screen for local player who has finished the race but is waiting for others to finish
void HUD::RenderRaceFinishedWaitingScreen()
{
	if (NetworkManagerClient::sInstance->DidFinishRace() == false) {
		mHasPlayedRaceOverSound = false; // reset
		return;
	}
	if (!mHasPlayedRaceOverSound) {
		SoundManager::sInstance->Play("Finish");
		SoundManager::sInstance->PlayMusic("../Assets/Sound/Music/Theme/Lobby.wav");
		mHasPlayedRaceOverSound = true;
	}
	// Full-screen black background (opaque) and centered white text
	sf::View defaultView = WindowManager::sInstance->getDefaultView();
	sf::Vector2f viewSize = defaultView.getSize();
	sf::RectangleShape background(viewSize);
	background.setPosition(0.f, 0.f);
	background.setFillColor(sf::Color(0, 0, 0, 255));
	WindowManager::sInstance->draw(background);

	sf::Text text;
	const string prompt = "You finished! Waiting for the other racers..";
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

	// Draw background
	sf::View defaultView = WindowManager::sInstance->getDefaultView();
	sf::Vector2f viewSize = defaultView.getSize();
	sf::RectangleShape background(viewSize);
	background.setPosition(0.f, 0.f);
	background.setFillColor(sf::Color(0, 0, 0, 255));
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
// Darren Meidl - D00255479 - Render wait screen for clients only
void HUD::RenderClientWaitingScreen() {
	// Display this screen only when lobby is open
	if (NetworkManagerClient::sInstance->IsLobbyOpen())
		return;
	// Do not display this screen if we are the host
	if (NetworkManagerClient::sInstance->GetPlayerId() == 1)
		return;

	if (in_race_)
		return;

	// Full-screen black background (opaque) and centered white text
	sf::View defaultView = WindowManager::sInstance->getDefaultView();
	sf::Vector2f viewSize = defaultView.getSize();
	sf::RectangleShape background(viewSize);
	background.setPosition(0.f, 0.f);
	background.setFillColor(sf::Color(0, 0, 0, 255));
	WindowManager::sInstance->draw(background);

	sf::Text text;
	const string prompt = "Waiting on Host to Start.";
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

// Darren Meidl - D00255479 - Update local player's checkpoint and lap progress for HUD display
void HUD::SetPlayerRaceProgress(int inCurrentCheckpointIndex, int inTotalCheckpoints, int inCurrentLap, int inLapsToWin)
{
	mPlayerCurrentCheckpointIndex = inCurrentCheckpointIndex;
	mPlayerTotalCheckpoints = inTotalCheckpoints;
	mPlayerCurrentLap = inCurrentLap;
	mPlayerLapsToWin = inLapsToWin;
}
void HUD::SetPlayerFinished(bool inFinished) {
	mPlayerHasFinished = inFinished;
}
// Darren Meidl - D00255479 - Render checkpoint and lap info for local player
void HUD::RenderHUD()
{
	// Prepare default view size / margins
	sf::View defaultView = WindowManager::sInstance->getDefaultView();
	sf::Vector2f viewSize = defaultView.getSize();

	// If no local player, nothing to render for these elements
	if (!NetworkManagerClient::sInstance || NetworkManagerClient::sInstance->GetPlayerId() == 0)
		return;

	// Obtain star count from ScoreBoardManager entry for local player (use score as star count)
	int playerStars = 0;
	uint32_t localPlayerId = NetworkManagerClient::sInstance->GetPlayerId();
	if (ScoreBoardManager::sInstance)
	{
		auto entry = ScoreBoardManager::sInstance->GetEntry(localPlayerId);
		if (entry)
		{
			playerStars = entry->GetScore();
		}
	}
	// cap at 20 as requested
	if (playerStars > 20) playerStars = 20;
	if (playerStars < 0) playerStars = 0;

	// Format star text as zero-padded two digits
	string starTextStr = StringUtils::Sprintf("%02d", playerStars);

	// Laps
	int displayLap = mPlayerCurrentLap + 1;
	int lapGoal = (mPlayerLapsToWin > 0 ? mPlayerLapsToWin : 0);
	string lapTextStr = StringUtils::Sprintf("%d/%d", displayLap, lapGoal);

	// Visual params
	const float marginRight = 40.f;
	const float marginLeft = 20.f;
	const float marginBottom = 30.f;
	const float padding = 8.f;
	const float innerSpacing = 8.f; // spacing between icon and text within an element
	const float elementGap = 20.f;  // space between star element and lap element
	const unsigned int charSize = 50; // match existing RenderText character size

	// Prepare SFML text objects
	sf::Text starText;
	starText.setFont(*FontManager::sInstance->GetFont("carlito"));
	starText.setString(starTextStr);
	starText.setCharacterSize(charSize);
	starText.setFillColor(sf::Color::White);

	sf::Text lapText;
	lapText.setFont(*FontManager::sInstance->GetFont("carlito"));
	lapText.setString(lapTextStr);
	lapText.setCharacterSize(charSize);
	lapText.setFillColor(sf::Color::White);

	// Measure text bounds
	sf::FloatRect starBounds = starText.getLocalBounds();
	sf::FloatRect lapBounds = lapText.getLocalBounds();
	float starTextW = starBounds.width;
	float starTextH = starBounds.height;
	float lapTextW = lapBounds.width;
	float lapTextH = lapBounds.height;

	// Load star texture (use as placeholder for both icons)
	auto starTex = TextureManager::sInstance->GetTexture("star");
	float iconW = 0.f, iconH = 0.f;
	sf::Sprite starSprite;
	if (starTex)
	{
		starSprite.setTexture(*starTex);
		sf::Vector2u tSize = starTex->getSize();
		// scale icon to approximately match text height
		iconH = static_cast<float>(charSize); // target height in pixels
		float scale = iconH / static_cast<float>(tSize.y);
		iconW = static_cast<float>(tSize.x) * scale;
		starSprite.setScale(scale, scale);
	}

	// Compute combined size: [icon + innerSpacing + text] for star element, same for lap element
	float starElemW = iconW + innerSpacing + starTextW;
	float lapElemW = iconW + innerSpacing + lapTextW;
	float combinedW = padding*2 + starElemW + elementGap + lapElemW;
	// Height is padding*2 + max of iconH and text heights (textH is bounds.height; we also account for bounds.top when positioning)
	float contentH = std::max(iconH, std::max(starTextH, lapTextH));
	float combinedH = padding*2 + contentH;

	// Background rect positioned bottom-left
	float bgX = marginLeft;
	float bgY = viewSize.y - marginBottom - combinedH;

	sf::RectangleShape background(sf::Vector2f(combinedW, combinedH));
	background.setPosition(bgX, bgY);
	background.setFillColor(sf::Color(0, 0, 0, 180)); // semi-transparent black
	WindowManager::sInstance->draw(background);

	// Draw star element
	float curX = bgX + padding;
	float innerY = bgY + padding;

	// star icon
	if (starTex)
	{
		// vertically center icon within content area
		float iconY = innerY + (contentH - iconH) / 2.f;
		starSprite.setPosition(curX, iconY);
		WindowManager::sInstance->draw(starSprite);
	}
	curX += iconW + innerSpacing;

	// star text: need to adjust for text local bounds top (font metrics)
	{
		float txtY = innerY + (contentH - starTextH) / 2.f - starBounds.top;
		starText.setPosition(curX, txtY);
		WindowManager::sInstance->draw(starText);
	}
	curX += starTextW;

	// gap between elements
	curX += elementGap;

	// Draw lap element (icon + text)
	if (starTex)
	{
		float iconY = innerY + (contentH - iconH) / 2.f;
		starSprite.setPosition(curX, iconY);
		WindowManager::sInstance->draw(starSprite);
	}
	curX += iconW + innerSpacing;

	{
		float txtY = innerY + (contentH - lapTextH) / 2.f - lapBounds.top;
		lapText.setPosition(curX, txtY);
		WindowManager::sInstance->draw(lapText);
	}

	// Darren Meidl - D00255479 - Determine local player's current position in the race
	int playerPosition = 0;
	// Compute live ordering by progress
	if (playerPosition == 0 && World::sInstance)
	{
		std::vector<std::pair<int, uint32_t>> progressList;
		const auto& gameObjects = World::sInstance->GetGameObjects();
		for (const auto& goPtr : gameObjects)
		{
			PlayerCar* car = goPtr->GetAsCar();
			if (car)
			{
				int lap = car->GetCurrentLap();
				int cpIndex = car->GetCurrentCheckpointIndex();
				if (cpIndex < 0) cpIndex = 0;
				int progress = lap * 10000 + cpIndex;
				progressList.emplace_back(progress, car->GetPlayerId());
			}
		}

		// sort descending by progress
		std::sort(progressList.begin(), progressList.end(), [](const auto& a, const auto& b) {
			return a.first > b.first;
		});

		for (size_t i = 0; i < progressList.size(); ++i)
		{
			if (progressList[i].second == localPlayerId)
			{
				playerPosition = static_cast<int>(i) + 1;
				break;
			}
		}
	}

	// Helper to get ordinal suffix (handles 11-13)
	auto GetOrdinalSuffix = [](int n) -> const char* {
		int mod100 = n % 100;
		if (mod100 >= 11 && mod100 <= 13) return "th";
		switch (n % 10)
		{
		case 1: return "st";
		case 2: return "nd";
		case 3: return "rd";
		default: return "th";
		}
	};

	if (playerPosition > 0 && playerPosition <= 15)
	{
		string posStr = StringUtils::Sprintf("%d%s", playerPosition, GetOrdinalSuffix(playerPosition));
		sf::Text posText;
		posText.setFont(*FontManager::sInstance->GetFont("carlito"));
		posText.setString(posStr);
		posText.setCharacterSize(charSize + 25);
		posText.setFillColor(sf::Color::White);

		sf::FloatRect pb = posText.getLocalBounds();
		// anchor bottom-right: set origin to bottom-right corner of the text bounds
		const float offsetFromEdge = 40.f;
		posText.setOrigin(pb.left + pb.width, pb.top + pb.height);
		posText.setPosition(viewSize.x - (marginRight + offsetFromEdge), viewSize.y - (marginBottom + offsetFromEdge));

		// Draw semi-transparent black circle behind the position text
		sf::FloatRect globalBounds = posText.getGlobalBounds();
		float centerX = globalBounds.left + globalBounds.width * 0.5f;
		float centerY = globalBounds.top + globalBounds.height * 0.5f;
		float radius = std::max(globalBounds.width, globalBounds.height) * 0.5f + 40.f;

		sf::CircleShape bgCircle(radius);
		bgCircle.setOrigin(radius, radius);
		bgCircle.setPosition(centerX, centerY);
		bgCircle.setFillColor(sf::Color(0, 0, 0, 180));

		WindowManager::sInstance->draw(bgCircle);
		WindowManager::sInstance->draw(posText);
	}
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

