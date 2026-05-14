#include "RoboCatPCH.hpp"
#include "SaveFileUtilities.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <format>


// Ruby White - D00255322
//sf::Color SaveFileUtilities::RandomHSVColour() {
//	return SaveFileUtilities::HSVToRGB(rand() % 255, (rand() % 50) + 50, 100);
//}
//
////converts hsv to an sf::color
////referencing https://axonflux.com/handy-rgb-to-hsl-and-rgb-to-hsv-color-model-c
//sf::Color SaveFileUtilities::HSVToRGB(float h, float s, float v) {
//	// algorithm assumes values are 0-1, so dividing to make that true
//	h /= 255.f;
//	s /= 100.f;
//	v /= 100.f;
//
//	float r = 0;
//	float g = 0;
//	float b = 0;
//
//	float i = floor(h * 6);
//	float f = h * 6 - i;
//	float p = v * (1 - s);
//	float q = v * (1 - f * s);
//	float t = v * (1 - (1 - f) * s);
//
//	switch ((int)i % 6) {
//	case 0: r = v, g = t, b = p; break;
//	case 1: r = q, g = v, b = p; break;
//	case 2: r = p, g = v, b = t; break;
//	case 3: r = p, g = q, b = v; break;
//	case 4: r = t, g = p, b = v; break;
//	case 5: r = v, g = p, b = q; break;
//	}
//
//	return sf::Color(r * 255, g * 255, b * 255);
//}
//
//sf::Color SaveFileUtilities::HexToRGB(std::string hexcode) {
//	if (hexcode[0] == '#') {
//		hexcode.erase(0, 1);
//	}
//	if (hexcode.length() == 6) {
//		sf::Color colour(HexToInt(hexcode.substr(0, 2)), HexToInt(hexcode.substr(2, 2)), HexToInt(hexcode.substr(4, 2)));
//		return colour;
//	}
//	else
//		return RandomHSVColour();
//}
//
//std::string SaveFileUtilities::RGBToHex(sf::Color& colour) {
//	std::string hex = std::format("{:x}", colour.r) + std::format("{:x}", colour.g) + std::format("{:x}", colour.b);
//
//	return hex;
//}
//
//uint8_t SaveFileUtilities::HexToInt(std::string hex) {
//	//https://stackoverflow.com/questions/1070497/c-convert-hex-string-to-signed-integer
//	unsigned int x;
//	std::stringstream ss;
//	ss << std::hex << hex;
//	ss >> x;
//	return (uint8_t)x;
//}

std::string SaveFileUtilities::GetUserNameFromFile() {

	//Try to open existing file
	std::ifstream input_file("Data/Username.txt");
	std::string name;
	if (input_file >> name) {
		if (name.length() > 0)
			return name;
	}

	//If the open/read failed or name too short, create a new file
	std::ofstream output_file("Data/Username.txt");
	std::string player = "Player";
	std::string new_name = player + std::to_string(rand()); // makes it so each random name is set to be Player{random numbers}
	output_file << new_name;
	return new_name;
}

//sf::Color SaveFileUtilities::GetUserColourFromFile() {
//	//Try to open existing file
//	std::ifstream input_file("Data/Colour.txt");
//	std::string hexcode;
//	if (input_file >> hexcode) {
//		if (hexcode.length() == 6)
//			return HexToRGB(hexcode);
//	}
//
//	//If the open/read failed, create a new file and a random colour
//	std::ofstream output_file("Data/Colour.txt");
//	sf::Color new_colour = RandomHSVColour();
//	output_file << RGBToHex(new_colour);
//	return new_colour;
//}

std::string SaveFileUtilities::GetAddressFromFile() {
	//Try to open existing file
	std::ifstream input_file("Data/IP.txt");
	std::string ip_address;
	if (input_file >> ip_address) {
		return ip_address;
	}

	//If the open/read failed, create a new file
	std::ofstream output_file("Data/IP.txt");
	std::string local_address = "127.0.0.1";
	output_file << local_address;
	return local_address;
}

std::string SaveFileUtilities::GetPortFromFile() {
	//Try to open existing file
	std::ifstream input_file("Data/Port.txt");
	std::string port;
	if (input_file >> port) {
		return port;
	}

	//If the open/read failed, create a new file and a random colour
	std::ofstream output_file("Data/Port.txt");
	std::string new_port = "50000";
	output_file << new_port;
	return new_port;
}