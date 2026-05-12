class Server : public Engine
{
public:

	static bool StaticInit();

	virtual void DoFrame() override;

	virtual int Run();

	void HandleNewClient(ClientProxyPtr inClientProxy);
	void HandleLostClient(ClientProxyPtr inClientProxy);

	PlayerCarPtr	GetCarForPlayer(int inPlayerId);
	void	SpawnCarForPlayer(int inPlayerId);


private:
	Server();

	bool	InitNetworkManager();
	void	SetupWorld();

};

