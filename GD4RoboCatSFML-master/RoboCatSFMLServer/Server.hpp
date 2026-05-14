class Server : public Engine
{
public:

	static bool StaticInit();

	virtual void DoFrame() override;

	virtual int Run();

	void HandleNewClient(ClientProxyPtr inClientProxy);
	void HandleLostClient(ClientProxyPtr inClientProxy);

	PlayerCarPtr GetCarForPlayer(int inPlayerId);
	void SpawnCarForPlayer(int inPlayerId, const Vector3& colour);


private:
	Server();

	bool InitNetworkManager();
	void SetupWorld();

	float mLobbyOpenStartTime = 0.f; // 0 == lobby not currently opened by round-end
	float mLobbyDuration = 5.0f;     // lobby open window in seconds (5s as requested)

	int checkpoint_count_;
};

