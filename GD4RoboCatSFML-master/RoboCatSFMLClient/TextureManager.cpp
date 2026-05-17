#include "RoboCatClientPCH.hpp"

std::unique_ptr< TextureManager >		TextureManager::sInstance;

void TextureManager::StaticInit()
{
	sInstance.reset(new TextureManager());
}

TextureManager::TextureManager()
{
	CacheTexture("car", "../Assets/Textures/Player/Car.png"); // Ruby White - D00255322 Darren Meidl - D00255479 - New car sprite
	CacheTexture("checkpoint", "../Assets/Textures/Checkpoint/checkpoint.png"); // Darren Meidl - D00255479 - New checkpoint sprite
	CacheTexture("star", "../Assets/Textures/Item/Star.png");  // Ruby White - D00255322 - new collectable sprite
	CacheTexture("flag", "../Assets/Textures/Item/Flag.png");  // Ruby White - D00255322 - icon for laps
	CacheTexture("track", "../Assets/Textures/World/Track.png");  // Ruby White - D00255322 - sprite for the track visuals
	CacheTexture("water", "../Assets/Textures/World/Water.png");  // Ruby White - D00255322 - sprite for background
	
}

TexturePtr	TextureManager::GetTexture(const string& inTextureName)
{
	return mNameToTextureMap[inTextureName];
}

bool TextureManager::CacheTexture(string inTextureName, const char* inFileName)
{
	TexturePtr newTexture(new sf::Texture());
	if (!newTexture->loadFromFile(inFileName))
	{
		return false;
	}

	mNameToTextureMap[inTextureName] = newTexture;

	return true;

}
