//I take care of rendering things!

class HUD
{
public:

	static void StaticInit();
	static std::unique_ptr< HUD >	sInstance;

	void Render();

	void SetPlayerHealth(int inHealth) { mHealth = inHealth; }

	// Race / checkpoint display for local player
	void SetPlayerRaceProgress(int inCurrentCheckpointIndex, int inTotalCheckpoints, int inCurrentLap, int inLapsToWin);

private:

	HUD();

	void RenderHostStartPrompt();
	void RenderLobbyWaitingScreen(); // new: render waiting-for-host screen for non-host players
	void RenderBandWidth();
	void RenderRoundTripTime();
	void RenderScoreBoard();
	void RenderRaceInfo();
	void RenderRaceOver();

	void RenderText(const string& inStr, const Vector3& origin, const Vector3& inColor);

	Vector3	mBandwidthOrigin;
	Vector3	mRoundTripTimeOrigin;
	Vector3	mScoreBoardOrigin;
	Vector3	mScoreOffset;
	Vector3	mRaceInfoOrigin;

	int	mHealth;
	// Darren Meidl - D00255479 - local player's race progress
	int	mPlayerCurrentCheckpointIndex;
	int	mPlayerTotalCheckpoints;
	int	mPlayerCurrentLap;
	int	mPlayerLapsToWin;
};



