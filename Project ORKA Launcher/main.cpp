//standard libraries
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <filesystem>

#include "curl/curl.h"

using String = std::string;
using Path = std::filesystem::path;
template<typename T>
using Vector = std::vector<T>;

template<typename T>
void log(T data)
{
	std::cout << data << "\n";
}

int main()
{
	Path orkaPath = std::filesystem::current_path();
	Path orkaDataPath = std::filesystem::current_path();
	Path vcpkgPath = Path(orkaPath.string().append("\\vcpkg"));
	String releasePath = "The-Engine\\x64\\Release\\";
	String debugPath = "The-Engine\\x64\\Debug\\";
	String libraryFileName = "libraries.txt";
	String solutionPath = String(orkaPath.string()).append("\\The-Engine");

	Vector<String> libs;

	std::ifstream inFile;

	inFile.open(libraryFileName);
	if(! inFile.is_open())
	{
		std::ofstream liboFile(libraryFileName);
		liboFile.close();
		inFile.open(libraryFileName);
	}

	if(! inFile.is_open())
	{
		log("Error");
	}

	String line;
	while(std::getline(inFile, line))
	{
		libs.push_back(line);
		log(line);
	}

	inFile.close();

	if(libs.size() == 0) log("No libraries used.");

	system("git clone https://github.com/Microsoft/vcpkg.git");

	std::filesystem::current_path(vcpkgPath);
	system("bootstrap-vcpkg.bat");

	system("vcpkg integrate install");

	system("vcpkg --disableMetrics");

	for(String& lib : libs)
	{
		system(String("vcpkg install ").append(lib).append(":x64-windows").c_str());
	}

	std::filesystem::current_path(orkaPath);

	system("git clone https://github.com/ProjectORKA/The-Engine.git");

	std::filesystem::create_directory("The-Engine");
	std::filesystem::create_directory("The-Engine\\x64");
	std::filesystem::create_directory("The-Engine\\x64\\Debug");
	std::filesystem::create_directory("The-Engine\\x64\\Release");

	Vector<Path> releasePaths;
	Vector<Path> debugPaths;
	for(const auto& file : std::filesystem::recursive_directory_iterator("vcpkg\\packages"))
	{
		bool isDll = file.path().extension() == ".dll";
		bool parentIsBinFolder = file.path().parent_path().filename() == "bin";
		bool isDebugDll = file.path().parent_path().parent_path().filename() == "debug";

		if(isDll && parentIsBinFolder)
		{
			if(isDebugDll) debugPaths.push_back(file.path());
			else releasePaths.push_back(file.path());
		}
	}

	for(Path& p : releasePaths)
	{
		try
		{
			std::filesystem::copy(p, String(releasePath).append(p.filename().string()), std::filesystem::copy_options::update_existing);
		}
		catch(std::filesystem::filesystem_error& e)
		{
			std::cout << "Error: " << e.what() << std::endl;
			std::cout << "This error could be resolved by restarting the application or manually dragging (" << String(releasePath).append(p.filename().string()) << ") into (" << p << "" << std::endl;
		}
	}

	for(Path& p : debugPaths)
	{
		try
		{
			std::filesystem::copy(p, String(debugPath).append(p.filename().string()), std::filesystem::copy_options::update_existing);
		}
		catch(std::filesystem::filesystem_error& e)
		{
			std::cout << "Error: " << e.what() << std::endl;
			std::cout << "This error could be resolved by restarting the application or manually dragging (" << String(releasePath).append(p.filename().string()) << ") into (" << p << "" << std::endl;
		}
	}

	std::filesystem::current_path(solutionPath);

	system("git pull");

	std::cout << "\a";

	system("\"Project ORKA.sln\"");

	std::cout << "Project ORKA was successfully installed. You can close this window now.";

	system("pause");
	return 0;
}