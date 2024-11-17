#include "Zipper.hpp"
#include <libzippp/libzippp.h>

void extractZip(const Path& archive, const Path& target)
{
    Path absolutePath = std::filesystem::absolute(archive);
    Path absoluteTarget = std::filesystem::absolute(target);

    // Open the ZIP archive
    libzippp::ZipArchive zipArchive(absolutePath.string());
    if (!zipArchive.open(libzippp::ZipArchive::ReadOnly)) {
        std::cerr << "Failed to open archive: " << absolutePath << std::endl;
        return;
    }

    // Get all entries in the archive
    Vector<libzippp::ZipEntry> entries = zipArchive.getEntries();
    for (const auto& entry : entries) {
        std::filesystem::path outputPath = absoluteTarget / entry.getName();

        if (entry.isDirectory()) {
            // Create directories
            std::filesystem::create_directories(outputPath);
        } else {
            // Ensure the parent directory exists
            std::filesystem::create_directories(outputPath.parent_path());

            // Open a stream to the output file
            std::ofstream outputFile(outputPath, std::ios::binary);
            if (!outputFile) {
                std::cerr << "Failed to create file: " << outputPath << std::endl;
                continue;
            }

            // Read the entry content and write it to the output file
            entry.readContent(outputFile);
            outputFile.close();
        }
    }

    // Close the ZIP archive
    zipArchive.close();
}