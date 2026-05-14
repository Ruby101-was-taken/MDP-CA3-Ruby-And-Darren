#include "RoboCatPCH.hpp"
#include "Logging.hpp"
#include <fstream>

std::string Logging::file_path_;
bool Logging::initialised_ = false;

void Logging::Log(std::string name, std::string message) {
	if (initialised_ or true) {
		std::ofstream log(file_path_, std::ios::app);
		log << "[" << name << "]: " << message << "\n";
	}
}

void Logging::LogInit() {

	file_path_ = "../Log.txt";

	ClearLog();

	initialised_ = true;
}

void Logging::ClearLog() {
	//https://stackoverflow.com/a/17033060
	std::ofstream ofs;
	ofs.open(file_path_, std::ofstream::out | std::ofstream::trunc);
	ofs.close();
}
