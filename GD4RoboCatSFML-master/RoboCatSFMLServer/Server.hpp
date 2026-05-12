class Server : public Engine
{
public:

	static bool StaticInit();

	virtual void DoFrame() override;

	virtual int Run();

	void HandleNewClient(ClientProxyPtr inClientProxy);
	void HandleLostClient(ClientProxyPtr inClientProxy);

	PlayerCarPtr GetCarForPlayer(int inPlayerId);
	void SpawnCarForPlayer(int inPlayerId);


private:
	Server();

	bool InitNetworkManager();
	void SetupWorld();
	// Darren Meidl - D00255479
	float mTimeOfGameOver = 0.f; // time when the server observed game-over (0 == not set)
	float mLobbyReturnDelay = 3.0f; // seconds to wait showing winners before returning to lobby
};

