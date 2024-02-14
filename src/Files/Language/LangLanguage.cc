#include "Global.h"
#include <GMLIB/Files/Language/LangLanguage.h>

namespace GMLIB::Files {

LangLanguage::LangLanguage(std::string& filePath, std::string& defaultLanguage) : mFilePath(filePath) {
    auto data = McLang::prase(defaultLanguage);
    merge_patch(data);
}

bool LangLanguage::init() {
    auto dirPath = std::filesystem::path(mFilePath).parent_path();
    if (!std::filesystem::exists(dirPath)) {
        std::filesystem::create_directories(dirPath);
    }
    if (std::filesystem::exists(mFilePath)) {
        auto oldData = McLang::prase_file(mFilePath);
        merge_patch(oldData);
    }
    return write_to_file(mFilePath);
}

} // namespace GMLIB::Files