#include "Program.hpp"
#include "curl/curl.h"

void Program::run() const
{
    installProjectOrka(orkaPath);

    Vector<String> libs = readLibrariesFromFile(launcherConfig);

    if (libs.empty()) std::cout << "No libraries used.\n";
    else
    {
        installVcpkg(vcpkgPath);
        installLibrariesSequentially(libs);
        //installLibrariesMultiThreaded(libs);
    }

    if (!libs.empty())
    {
        Vector<Path> releasePaths;
        Vector<Path> debugPaths;
        for (const auto& file : std::filesystem::recursive_directory_iterator(vcpkgPackagesPath))
        {
            const Bool isDll = file.path().extension() == ".dll";
            const Bool parentIsBinFolder = file.path().parent_path().filename() == "bin";
            const Bool isDebugDll = file.path().parent_path().parent_path().filename() == "debug";

            if (isDll && parentIsBinFolder)
            {
                if (isDebugDll) debugPaths.push_back(file.path());
                else releasePaths.push_back(file.path());
            }
        }

        for (Path& p : releasePaths)
        {
            try
            {
                std::filesystem::copy(p, releaseVersionPath.string().append(p.filename().string()),
                                      std::filesystem::copy_options::update_existing);
            }
            catch (std::filesystem::filesystem_error& e)
            {
                std::cout << "Error: " << e.what() << std::endl;
                std::cout << "This error could be resolved by restarting the application or manually dragging (" <<
                    releaseVersionPath.string().append(p.filename().string()) << ") into (" << p << "" << std::endl;
            }
        }

        for (Path& p : debugPaths)
        {
            try
            {
                std::filesystem::copy(p, debugVersionPath.string().append(p.filename().string()),
                                      std::filesystem::copy_options::update_existing);
            }
            catch (std::filesystem::filesystem_error& e)
            {
                std::cout << "Error: " << e.what() << std::endl;
                std::cout << "This error could be resolved by restarting the application or manually dragging (" <<
                    releaseVersionPath.string().append(p.filename().string()) << ") into (" << p << "" << std::endl;
            }
        }
    }

    setCurrentPath(orkaPath);

    createJunction(debugLinkDataPath.string(), dataPath.string());
    createJunction(releaseLinkDataPath.string(), dataPath.string());
    createJunction(debugLinkCachePath.string(), cachePath.string());
    createJunction(releaseLinkCachePath.string(), cachePath.string());

    if (downloadFile(projectOrkaDataUrl, dataDownloadFile.string())) std::cout <<
        "... Files downloaded successfully.\n";
    else std::cerr << "Failed to download and unpack file." << std::endl;

    extractZip("Data.zip", dataPath);

    std::filesystem::remove("Data.zip");

    // mklink /D Data ..\..\"Project ORKA"\Data
    // mklink /D Cache ..\..\"Project ORKA"\Cache

    openSolution(solutionPath);
}

void executeProgram(const String& command)
{
    std::cout << "\n\n RUNNING COMMAND: " + command + "\n\n";

    const int bufferSize = MultiByteToWideChar(CP_UTF8, 0, command.c_str(), -1, nullptr, 0);
    if (bufferSize == 0)
    {
        std::cerr << "Error getting buffer size.\n";
        return;
    }

    std::wstring wideCommand(bufferSize, L'\0');
    if (MultiByteToWideChar(CP_UTF8, 0, command.c_str(), -1, wideCommand.data(), bufferSize) == 0)
    {
        std::cerr << "Error converting to wide-character string.\n";
        return;
    }

    STARTUPINFO startupInfo;
    PROCESS_INFORMATION processInfo;

    ZeroMemory(&startupInfo, sizeof startupInfo);
    startupInfo.cb = sizeof startupInfo;
    ZeroMemory(&processInfo, sizeof processInfo);

    if (!CreateProcess(nullptr, wideCommand.data(), nullptr, nullptr, FALSE, 0, nullptr, nullptr, &startupInfo,
                       &processInfo))
    {
        const DWORD errorCode = GetLastError();
        LPVOID errorMessage;
        if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, nullptr, errorCode, 0,
                          reinterpret_cast<LPWSTR>(&errorMessage), 0, nullptr))
        {
            std::wcerr << "CreateProcess failed: " << static_cast<LPWSTR>(errorMessage) << "\n";
            LocalFree(errorMessage);
        }
        else std::wcerr << "CreateProcess failed with error code: " << errorCode << "\n";

        return;
    }

    WaitForSingleObject(processInfo.hProcess, INFINITE);

    CloseHandle(processInfo.hProcess);
    CloseHandle(processInfo.hThread);
}

void installLibrary(const String& lib)
{
    std::cout << "Installing (" + lib + ")\n";

    const String installCommand = "vcpkg install " + lib + +":x64-windows";
    std::cout << installCommand << "\n";
    executeProgram(installCommand);

    std::cout << "Installed (" + lib + ")\n";
}

void Program::installLibrariesSequentially(Vector<String>& libs) const
{
    for (String& lib : libs) installLibrary(lib);
}

void Program::installLibrariesMultiThreaded(const Vector<String>& libs) const
{
    std::cout << "Install libraries multithreaded.\n";

    Vector<Thread> threads;
    for (const String& lib : libs) threads.emplace_back(installLibrary, lib);
    for (Thread& t : threads) t.join();

    std::cout << "Installed libraries.\n";
}

void Program::createJunction(const String& junctionPath, const String& targetPath) const
{
    system(String("mklink /J \"" + junctionPath + "\" \"" + targetPath + "\"").c_str());
}

Vector<String> Program::readLibrariesFromFile(const Path& librariesFile) const
{
    Vector<String> libs;
    std::ifstream inFile;
    inFile.open(librariesFile);
    if (!inFile.is_open())
    {
        std::ofstream libOutFile(librariesFile);
        libOutFile.close();
        inFile.open(librariesFile);
    }
    if (!inFile.is_open()) std::cout << "Error\n";

    String line;
    while (std::getline(inFile, line))
    {
        libs.push_back(line);
        std::cout << line << "\n";
    }
    inFile.close();
    return libs;
}

void Program::installVcpkg(const Path& vcpkgPath) const
{
    std::cout << "Installing vcpkg ...\n";

    setCurrentPath(programFilesPath);
    system("git clone https://github.com/Microsoft/vcpkg.git");
    setCurrentPath(vcpkgPath);

    system("git pull");
    system("vcpkg upgrade");
    system("bootstrap-vcpkg.bat");
    system("vcpkg integrate install");
    system("vcpkg --disableMetrics");

    std::cout << "vcpkg installed...\n";
}

void Program::installProjectOrka(const Path& orkaPath) const
{
    setCurrentPath(orkaPath);

    system("git clone https://github.com/ProjectORKA/The-Engine.git");

    createDirectory("The-Engine");
    createDirectory("The-Engine\\x64");
    createDirectory("The-Engine\\x64\\Debug");
    createDirectory("The-Engine\\x64\\Release");
    createDirectory(dataPath);
    createDirectory(cachePath);

    system("git pull");
}

void Program::openSolution(const Path& solutionPath) const
{
    setCurrentPath(solutionPath);
    system("\"Project ORKA.sln\"");
    std::cout << "\a";
}

static SizeT writeCallback(void* contents, const SizeT byteSize, const SizeT nmemb, void* userp)
{
    const SizeT totalSize = byteSize * nmemb;
    std::ostream& stream = *static_cast<std::ostream*>(userp);
    stream.write(static_cast<char*>(contents), static_cast<long long>(totalSize));
    return totalSize;
}

Bool Program::downloadFile(const String& url, const String& outputPath) const
{
    std::cout << "Downloading File (" + url + ")\n";

    CURL* curl = curl_easy_init();
    if (!curl)
    {
        std::cerr << "Failed to initialize libcurl." << std::endl;
        return false;
    }

    std::ofstream outputFile(outputPath, std::ios::binary);
    if (!outputFile.is_open())
    {
        std::cerr << "Failed to open output file." << std::endl;
        return false;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &outputFile);

    const CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK)
    {
        std::cerr << "Failed to download file: " << curl_easy_strerror(res) << std::endl;
        return false;
    }

    curl_easy_cleanup(curl);
    outputFile.close();
    std::cout << "Downloaded File (" + url + ")\n";
    return true;
}
