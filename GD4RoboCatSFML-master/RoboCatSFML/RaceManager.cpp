#include "RoboCatPCH.hpp"
// Darren Meidl - D000255479 - Entire class
std::unique_ptr<RaceManager> RaceManager::sInstance;

void RaceManager::StaticInit()
{
    sInstance.reset(new RaceManager());
}

RaceManager::RaceManager()
{
}

void RaceManager::AddPlayer(uint32_t inPlayerId)
{
    mActivePlayers.insert(inPlayerId);
}

void RaceManager::RemovePlayer(uint32_t inPlayerId)
{
    mActivePlayers.erase(inPlayerId);

    if (AreAllPlayersFinished())
    {
        if (ScoreBoardManager::sInstance)
        {
            ScoreBoardManager::sInstance->SetFinishers(mFinishOrder);
        }
    }
}

void RaceManager::OnPlayerFinished(PlayerCar* inCar)
{
    if (!inCar) return;
    uint32_t pid = inCar->GetPlayerId();
    // avoid double-counting
    if (std::find(mFinishOrder.begin(), mFinishOrder.end(), pid) != mFinishOrder.end())
        return;
  
    mFinishOrder.push_back(pid); // record finish order

    inCar->SetDoesWantToDie(true); // mark the car for removal on the server
    // Update latest finishers list
    if (ScoreBoardManager::sInstance)
        ScoreBoardManager::sInstance->SetFinishersOnly(mFinishOrder);
    // if everyone is finished, finalize finishers
    if (ScoreBoardManager::sInstance && AreAllPlayersFinished())
        ScoreBoardManager::sInstance->SetFinishers(mFinishOrder);
}

bool RaceManager::AreAllPlayersFinished() const
{
    if (mActivePlayers.empty())
        return true;

    for (uint32_t pid : mActivePlayers)
    {
        if (std::find(mFinishOrder.begin(), mFinishOrder.end(), pid) == mFinishOrder.end())
            return false;
    }
    return true;
}

void RaceManager::Reset()
{
    // Clear finish order and active players so a new race can begin cleanly.
    mFinishOrder.clear();
    mActivePlayers.clear();

    // Clear game-over state on the scoreboard so clients see lobby again.
    if (ScoreBoardManager::sInstance)
    {
        ScoreBoardManager::sInstance->SetGameOver(false);
    }
}