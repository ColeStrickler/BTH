#include "filebrowser.h"

FileBrowser::FileBrowser() : m_InputPath("")
{
}

std::vector<fs::directory_entry> FileBrowser::ListDirectory(const std::string& path)
{
   // auto& directory_contents = m_CurrentDirectory;

    fs::path inputPath = fs::path(path);
    if (!fs::exists(inputPath) || !fs::is_directory(inputPath)) {
        return m_CurrentDirectory;
    }

    m_CurrentDirectory.clear();

    for (const auto& entry : fs::directory_iterator(inputPath)) {
        auto path = entry.path();
        m_CurrentDirectory.push_back(entry);
    }


    return m_CurrentDirectory;
}

std::vector<std::string> FileBrowser::PathToStringVec(const std::vector<fs::path>& pvec) const
{
    std::vector<std::string> ret;

    for (auto& e : pvec)
    {
        ret.push_back(e.string());
    }
    return ret;
}

std::vector<std::string> FileBrowser::CurrentDirectoryToStringVec()
{
    std::vector<std::string> ret;

    for (auto& e : m_CurrentDirectory)
    {
        ret.push_back(e.path().string());
    }
    return ret;

}

std::vector<std::string> FileBrowser::DisplayFilter(std::vector<std::string>& unfiltered)
{
    std::vector<std::string> ret;
    std::string input(m_InputPath);

    for (auto& s : unfiltered)
    {
        if (s.size() < input.size())
        {
            continue;
        }
        bool skip = false;
        for (int i = 0; i < input.size(); i++)
        {
            if (input[i] != s[i])
            {
                skip = true;
                break;
            }
        }
        if (!skip)
        {
            ret.push_back(s);
        }
        return ret;
    }


}

unsigned char* FileBrowser::LoadNewFile(const std::string& filepath)
{
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        return nullptr;
    }
    m_FileLoadData.clear();

    
    // Get the file size for reserving space in the vector
    file.seekg(0, std::ios::end);
    std::streampos fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    // Reserve space in the vector
    m_FileLoadData.reserve(static_cast<size_t>(fileSize));

    // Read the entire file into the vector
    m_FileLoadData.insert(m_FileLoadData.begin(),
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>());
    return nullptr;
}

void FileBrowser::SetInputPath(const std::string& new_inputpath)
{
    memset(m_InputPath, 0x00, _MAX_PATH);
    memcpy(m_InputPath, new_inputpath.c_str(), sizeof(char) * new_inputpath.size());
}

