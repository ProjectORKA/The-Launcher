
//standard libraries
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <filesystem>

//#include <Windows.h>

//#include <zip.h>
#include "curl/curl.h"

//set up datatypes
using String = std::string;
using Path = std::filesystem::path;
template<typename T>
using Vector = std::vector<T>;

//int unpackGameFiles(Path filePath) {
//
//
//	int err;
//	struct zip* z;
//
//	z = zip_open(filePath.string().c_str(), 0, &err);
//	if (!z) {
//		std::cerr << "Error opening zip archive: " << zip_strerror(z) << std::endl;
//		return -1;
//	}
//
//	for (int i = 0; i < zip_get_num_entries(z, 0); i++) {
//		struct zip_file* f;
//		struct zip_stat s;
//
//		if (zip_stat_index(z, i, 0, &s) == 0) {
//			f = zip_fopen_index(z, i, 0);
//			if (!f) {
//				std::cerr << "Error opening file in zip archive: " << zip_strerror(z) << std::endl;
//				continue;
//			}
//
//			std::ofstream outfile(s.name, std::ofstream::binary);
//			char buf[1024];
//			int bytes_read;
//			while ((bytes_read = zip_fread(f, buf, sizeof(buf))) > 0) {
//				outfile.write(buf, bytes_read);
//			}
//
//			outfile.close();
//			zip_fclose(f);
//		}
//	}
//
//	zip_close(z);
//}

//void downloadFromUrl(String url) {
//	CURL* curl = nullptr;
//	FILE* fp = nullptr;
//	CURLcode res;
//	char outfilename[FILENAME_MAX] = "file.txt";
//	curl = curl_easy_init();
//	if (curl) {
//		fp = fopen(outfilename, "wb");
//		curl_easy_setopt(curl, CURLOPT_URL, url);
//		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
//		curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
//		res = curl_easy_perform(curl);
//		/* always cleanup */
//		curl_easy_cleanup(curl);
//		fclose(fp);
//	}
//}




template<typename T>
void log(T data) {
	std::cout << data << "\n";
}

int main() {
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
	if (!inFile.is_open())
	{
		std::ofstream liboFile(libraryFileName);
		liboFile.close();
		inFile.open(libraryFileName);
	}

	if (!inFile.is_open()) {
		log("Error");
	}

	String line;
	while (std::getline(inFile, line))
	{
		libs.push_back(line);
		log(line);
	}

	inFile.close();

	if (libs.size() == 0) log("No libraries used.");

	system("git clone https://github.com/Microsoft/vcpkg.git");

	std::filesystem::current_path(vcpkgPath);
	system("bootstrap-vcpkg.bat");

	system("vcpkg integrate install");

	for (String& lib : libs) {
		system(String("vcpkg install ").append(lib).append(":x64-windows").c_str());
	}

	std::filesystem::current_path(orkaPath);

	system("git clone https://github.com/ProjectORKA/The-Engine.git");

	//create output directories already
	std::filesystem::create_directory("The-Engine");
	std::filesystem::create_directory("The-Engine\\x64");
	std::filesystem::create_directory("The-Engine\\x64\\Debug");
	std::filesystem::create_directory("The-Engine\\x64\\Release");

	//get all dlls
	Vector<Path> releasePaths;
	Vector<Path> debugPaths;
	for (const auto& file : std::filesystem::recursive_directory_iterator("vcpkg\\packages")) {
		bool isDll = file.path().extension() == ".dll";
		bool parentIsBinFolder = file.path().parent_path().filename() == "bin";
		bool isDebugDll = file.path().parent_path().parent_path().filename() == "debug";

		if (isDll && parentIsBinFolder) {
			if (isDebugDll) debugPaths.push_back(file.path());
			else releasePaths.push_back(file.path());
		}
	}

	//and copy them into the executable paths
	for (Path& p : releasePaths) {
		try {
			std::filesystem::copy(p, String(releasePath).append(p.filename().string()), std::filesystem::copy_options::update_existing);
		}
		catch (std::filesystem::filesystem_error& e) {
			std::cout << "Error: " << e.what() << std::endl;
			std::cout << "This error could be resolved by restarting the application or manually dragging (" << String(releasePath).append(p.filename().string()) << ") into (" << p << "" << std::endl;
		}
	}

	for (Path& p : debugPaths) {
		try {
			std::filesystem::copy(p, String(debugPath).append(p.filename().string()), std::filesystem::copy_options::update_existing);
		}
		catch (std::filesystem::filesystem_error& e) {
			std::cout << "Error: " << e.what() << std::endl;
			std::cout << "This error could be resolved by restarting the application or manually dragging (" << String(releasePath).append(p.filename().string()) << ") into (" << p << "" << std::endl;
		}
	}

	std::filesystem::current_path(solutionPath);

	system("git pull");

	std::cout << "\a";

	system("\"Project ORKA.sln\"");


	//downloadFromUrl("https://www.project-orka.com/files/ORKA%20Data.zip");

	//system("curl https://www.project-orka.com/files/ORKA%20Data.zip --output \"Data.zip\"");

	//unpackGameFiles();
	std::cout << "Project ORKA was sucessfully installed. You can close this window now.";

	system("pause");
	return 0;
}