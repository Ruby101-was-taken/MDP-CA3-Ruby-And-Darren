#pragma once
class SaveFileUtilities {
public:
	// Ruby White - D00255322 - whole class

	static std::string GetUserNameFromFile();
	static std::string GetAddressFromFile();
	static std::string GetPortFromFile();
	static bool CheckIfFolderExists(const char* dir);

	static void MakeNeededFiles();

	static std::string sUsernamePath;
	static std::string sPortPath;
	static std::string sIPPath;
};

