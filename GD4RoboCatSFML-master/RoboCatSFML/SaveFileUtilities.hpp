#pragma once
class SaveFileUtilities {
public:
	// Ruby White - D00255322
	//static sf::Color RandomHSVColour();
	//static sf::Color HSVToRGB(float h, float s, float v);
	//static sf::Color HexToRGB(std::string hexcode);

	//static std::string RGBToHex(sf::Color& colour);
	//static uint8_t HexToInt(std::string hex);

	static std::string GetUserNameFromFile();
	//static sf::Color GetUserColourFromFile();
	static std::string GetAddressFromFile();
	static std::string GetPortFromFile();
};

