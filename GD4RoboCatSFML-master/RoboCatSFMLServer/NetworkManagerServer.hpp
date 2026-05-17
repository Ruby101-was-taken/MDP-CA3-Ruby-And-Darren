class NetworkManagerServer : public NetworkManager
{
public:
	static NetworkManagerServer* sInstance;

	static bool	StaticInit(uint16_t inPort);

	virtual void ProcessPacket(InputMemoryBitStream& inInputStream, const SocketAddress& inFromAddress) override;
	virtual void HandleConnectionReset(const SocketAddress& inFromAddress) override;
	// Darren Meidl - D00255479 - Lobby / game state control
	// When true, server accepts new joining players (lobby). When false, reject new joins (game in progress).
	void SetIsInLobby(bool inIsInLobby) { mIsInLobby = inIsInLobby; }
	bool IsInLobby() const { return mIsInLobby; }

	void SendOutgoingPackets();
	void CheckForDisconnects();
	// Get list of currently connected player IDs
	std::vector<int> GetConnectedPlayerIds() const;

	void RegisterGameObject(GameObjectPtr inGameObject);
	inline GameObjectPtr RegisterAndReturn(GameObject* inGameObject);
	void UnregisterGameObject(GameObject* inGameObject);
	void SetStateDirty(int inNetworkId, uint32_t inDirtyState);

	void RespawnCats();

	ClientProxyPtr GetClientProxy(int inPlayerId) const;

	void UpdateAllClients();
private:
	NetworkManagerServer();

	void HandlePacketFromNewClient(InputMemoryBitStream& inInputStream, const SocketAddress& inFromAddress);
	void ProcessPacket(ClientProxyPtr inClientProxy, InputMemoryBitStream& inInputStream);

	void SendWelcomePacket(ClientProxyPtr inClientProxy);
	void SendLobbyStatePacket(ClientProxyPtr inClientProxy); // Darren Meidl - D00255479 - Lobby state packet
	void HandleClientDisconnected(ClientProxyPtr inClientProxy);

	void AddWorldStateToPacket(OutputMemoryBitStream& inOutputStream);
	void AddScoreBoardStateToPacket(OutputMemoryBitStream& inOutputStream);

	void SendStatePacketToClient(ClientProxyPtr inClientProxy);
	void WriteLastMoveTimestampIfDirty(OutputMemoryBitStream& inOutputStream, ClientProxyPtr inClientProxy);

	void HandleInputPacket(ClientProxyPtr inClientProxy, InputMemoryBitStream& inInputStream);

	int GetNewNetworkId();

	typedef unordered_map< int, ClientProxyPtr >	IntToClientMap;
	typedef unordered_map< SocketAddress, ClientProxyPtr >	AddressToClientMap;

	AddressToClientMap mAddressToClientMap;

	int mNewPlayerId;
	int mNewNetworkId;

	float mTimeOfLastSatePacket;
	float mTimeBetweenStatePackets;
	float mClientDisconnectTimeout;
	
	bool mIsInLobby;

protected:
	IntToClientMap mPlayerIdToClientMap;
};


inline GameObjectPtr NetworkManagerServer::RegisterAndReturn(GameObject* inGameObject)
{
	GameObjectPtr toRet(inGameObject);
	RegisterGameObject(toRet);
	return toRet;
}


