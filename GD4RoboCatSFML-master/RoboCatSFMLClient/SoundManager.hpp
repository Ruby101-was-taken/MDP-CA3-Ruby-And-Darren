#pragma once
typedef shared_ptr< sf::Sound  > SoundPtr;
class SoundManager {
public:
	static void StaticInit();

	static std::unique_ptr<SoundManager> sInstance;

	void Play(std::string effect);

	void RemoveStoppedSounds();

	void SetVolume(float volume);
	float GetVolume();
	void IncrementVolume(float volume);

private:

	SoundManager();

	void Load(std::string id, const std::string& filename);

	std::map < std::string, sf::SoundBuffer > resource_map_;
	std::list<sf::Sound> sounds_;
	float volume_;
};
