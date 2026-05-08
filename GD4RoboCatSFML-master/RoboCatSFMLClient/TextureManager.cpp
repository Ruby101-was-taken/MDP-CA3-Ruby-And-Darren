#include "RoboCatClientPCH.hpp"

std::unique_ptr< TextureManager >		TextureManager::sInstance;

void TextureManager::StaticInit()
{
	sInstance.reset(new TextureManager());
}

TextureManager::TextureManager()
{
	CacheTexture("cat", "../Assets/cat.png");
	CacheTexture("car", "../Assets/car.png"); // Darren Meidl - D00255479 - New car sprite
	CacheTexture("checkpoint", "../Assets/checkpoint.png"); // Darren Meidl - D00255479 - New checkpoint sprite
	CacheTexture("checkpoint_passed", "../Assets/checkpoint_passed.png"); // Darren Meidl - D00255479 - New checkpoint passed sprite
	CacheTexture("checkpoint_next", "../Assets/checkpoint_next.png"); // NEW: next checkpoint sprite
	CacheTexture("mouse", "../Assets/mouse.png");
	CacheTexture("yarn", "../Assets/yarn.png");
	
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
