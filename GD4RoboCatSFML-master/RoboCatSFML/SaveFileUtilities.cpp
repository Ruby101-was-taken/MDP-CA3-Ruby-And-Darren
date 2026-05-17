#include "RoboCatPCH.hpp"
#include "SaveFileUtilities.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <format>
#include <filesystem>

std::string SaveFileUtilities::sUsernamePath;
std::string SaveFileUtilities::sPortPath;
std::string SaveFileUtilities::sIPPath;

std::string SaveFileUtilities::GetUserNameFromFile() {

	//Try to open existing file
	std::ifstream input_file(sUsernamePath);
	std::string name;
	if (input_file >> name) {
		if (name.length() > 0)
			return name;
	}

	//If the open/read failed or name too short, create a new file
	std::ofstream output_file(sUsernamePath);
	std::string player = "Player";
	std::string new_name = player + std::to_string(rand()); // makes it so each random name is set to be Player{random numbers}
	output_file << new_name;
	return new_name;
}

std::string SaveFileUtilities::GetAddressFromFile() {
	//Try to open existing file
	std::ifstream input_file(sIPPath);
	std::string ip_address;
	if (input_file >> ip_address) {
		return ip_address;
	}

	//If the open/read failed, create a new file
	std::ofstream output_file(sIPPath);
	std::string local_address = "127.0.0.1";
	output_file << local_address;
	return local_address;
}

std::string SaveFileUtilities::GetPortFromFile() {
	//Try to open existing file
	std::ifstream input_file(sPortPath);
	std::string port;
	if (input_file >> port) {
		return port;
	}

	//If the open/read failed, create a new file and a random colour
	std::ofstream output_file(sPortPath);
	std::string new_port = "50000";
	output_file << new_port;
	return new_port;
}

//https://www.geeksforgeeks.org/cpp/how-to-check-a-file-or-directory-exists-in-cpp/
bool SaveFileUtilities::CheckIfFolderExists(const char* dir) {

	// Structure which would store the metadata
	struct stat sb;

	// Calls the function with path as argument
	// If the file/directory exists at the path returns 0
	// If block executes if path exists
	return stat(dir, &sb) == 0;
}

void SaveFileUtilities::MakeNeededFiles() {

	sUsernamePath = "..\\Data\\Username.txt";
	sPortPath = "..\\Data\\Port.txt";
	sIPPath = "..\\Data\\IP.txt";

	if (not SaveFileUtilities::CheckIfFolderExists("../Data")) {
		std::filesystem::create_directories("../Data");
	}
	// load the files to make sure that they are existing and such
	SaveFileUtilities::GetAddressFromFile();
	SaveFileUtilities::GetPortFromFile();
	SaveFileUtilities::GetUserNameFromFile();
}
