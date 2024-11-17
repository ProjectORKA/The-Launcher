#pragma once

#include "Zipper.hpp"

struct Program
{
    void run() const;

private:
    Path orkaPath = getCurrentPath();

    Path programFilesPath = R"(C:\Program Files)";
    Path vcpkgPath = R"(C:\Program Files\vcpkg)";
    Path vcpkgPackagesPath = R"(C:\Program Files\vcpkg\packages)";

    Path dataDownloadFile = getCurrentPath().append(R"(Data.zip)");
    Path solutionPath = getCurrentPath().append(R"(The-Engine)");
    Path launcherConfig = getCurrentPath().append(R"(The-Engine\libraries.txt)");
    Path debugVersionPath = getCurrentPath().append(R"(The-Engine\x64\Debug\)");
    Path releaseVersionPath = getCurrentPath().append(R"(The-Engine\x64\Release\)");
    Path dataPath = getCurrentPath().append(R"(The-Engine\Project ORKA\Data)");
    Path debugLinkDataPath = getCurrentPath().append(R"(The-Engine\x64\Debug\Data)");
    Path releaseLinkDataPath = getCurrentPath().append(R"(The-Engine\x64\Release\Data)");
    Path cachePath = getCurrentPath().append(R"(The-Engine\Project ORKA\Cache)");
    Path debugLinkCachePath = getCurrentPath().append(R"(The-Engine\x64\Debug\Cache)");
    Path releaseLinkCachePath = getCurrentPath().append(R"(The-Engine\x64\Release\Cache)");
    String projectOrkaDataUrl = "https://project-orka.com/files/Data.zip";

    void installVcpkg(const Path& vcpkgPath) const;
    void openSolution(const Path& solutionPath) const;
    void installProjectOrka(const Path& orkaPath) const;
    void installLibrariesSequentially(Vector<String>& libs) const;
    void installLibrariesMultiThreaded(const Vector<String>& libs) const;
    void createJunction(const String& junctionPath, const String& targetPath) const;

    [[nodiscard]] Vector<String> readLibrariesFromFile(const Path& librariesFile) const;
    [[nodiscard]] Bool downloadFile(const String& url, const String& outputPath) const;
};

void installLibrary(const String& lib);
void executeProgram(const String& command);
