#include "RoboCatClientPCH.hpp"

std::unique_ptr< SoundManager > SoundManager::sInstance;

void SoundManager::StaticInit() {
	sInstance.reset(new SoundManager());
}

SoundManager::SoundManager()
	: volume_(100.f) {
	Load("StarGet", "../Assets/Sound/Star/Star-Get.ogg");

	Load("Lap", "../Assets/Sound/Music/Jingle/Lap.wav");
	Load("Finish", "../Assets/Sound/Music/Jingle/Finish.wav");

}

void SoundManager::Play(std::string effect) {
	sounds_.emplace_back(resource_map_[effect]);
	sf::Sound& sound = sounds_.back();

	sound.setVolume(volume_);

	sound.play();
}

void SoundManager::PlayMusic(std::string music_path) {
	if (music_.openFromFile(music_path)) {
		music_.play();
		music_.setLoop(true);
	}
}

void SoundManager::RemoveStoppedSounds() {
	sounds_.remove_if([](const sf::Sound& s) {
		return s.getStatus() == sf::Sound::Status::Stopped;
		});
}

void SoundManager::SetVolume(float volume) {
	volume_ = volume;
	volume_ = std::clamp(volume_, 0.f, 200.f);
}
float SoundManager::GetVolume() {
	return volume_;
}
void SoundManager::IncrementVolume(float volume) {
	volume_ += volume;
	volume_ = std::clamp(volume_, 0.f, 200.f);
}

void SoundManager::Load(std::string id, const std::string& filename) {
    sf::SoundBuffer resource;

    if (!resource.loadFromFile(filename))
        throw std::runtime_error(
            "ResourceHolder::Load - Failed to load " + filename
        );

    auto inserted = resource_map_.emplace(std::move(id), std::move(resource));
    assert(inserted.second);
}

