#pragma once
// Darren Meidl - D000255479 - Entire class
class RaceManager
{
public:
    static void StaticInit();
    static std::unique_ptr<RaceManager> sInstance;

    RaceManager();
    ~RaceManager() = default;

    // Register/unregister active players
    void AddPlayer(uint32_t inPlayerId);
    void RemovePlayer(uint32_t inPlayerId);
    
    void OnPlayerFinished(PlayerCar* inCar); // Called when a player's car completes the race

    // Query finish order
    const std::vector<uint32_t>& GetFinishOrder() const { return mFinishOrder; }
    bool AreAllPlayersFinished() const;

    // Reset per-round state (clear finish order, prepare for next round)
    void Reset();

private:
    std::unordered_set<uint32_t> mActivePlayers; // players still racing (or registered)
    std::vector<uint32_t> mFinishOrder; // in crossing order, front = 1st
};