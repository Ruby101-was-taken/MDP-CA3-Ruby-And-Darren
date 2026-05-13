#pragma once
class Logging {
public:
	static void Log(std::string name, std::string message);
	static void LogInit();
	static void ClearLog();

private:
	static std::string file_path_;
	static bool initialised_;
};

