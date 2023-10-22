#include "filebrowser.h"

FileBrowser::FileBrowser(size_t MaxLoadSize) : m_InputPath(""), m_MaxLoadSize(MaxLoadSize)
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

std::vector<fs::directory_entry> FileBrowser::DisplayFilter()
{
    std::vector<fs::directory_entry> ret;
    std::string input(m_InputPath);

    for (auto& s : m_CurrentDirectory)
    {
        auto str = s.path().string();
        if (str.size() < input.size())
        {
            continue;
        }
        bool skip = false;
        for (int i = input.size()-1; i >= 0; i--)
        {
            if (str[i] != input[i])
            {
                skip = true;
                break;
            }
        }
        if (!skip)
        {
            ret.push_back(s);
        }
    }

    return ret;
}

FB_RETCODE FileBrowser::LoadFile(const std::string& filepath, const size_t& offset)
{
    if (m_LoadedFileName == filepath && filepath.size() && offset >= m_CurrentBounds[0] && offset < m_CurrentBounds[1])
    {
        return FB_RETCODE::NO_CHANGE_LOAD;
    }

    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        return FB_RETCODE::FILE_ERROR_LOAD;
    }
    

    
    // Get the file size for reserving space in the vector
    
    file.seekg(0, std::ios::end);
    std::streampos fileSize = file.tellg();
    if (offset > fileSize)
    {
        m_CurrentBounds[0] = 0;
        m_CurrentBounds[1] = 1;
        // just reset
        file.close();
        return FB_RETCODE::FILE_ERROR_LOAD;
    }
    file.seekg(offset, std::ios::beg);



    FB_RETCODE RETURN_CODE = (m_LoadedFileName == filepath ? FB_RETCODE::OFFSET_CHANGE_LOAD : FB_RETCODE::FILE_CHANGE_LOAD);
    unsigned long loadSize = (static_cast<size_t>(fileSize) - offset) > m_MaxLoadSize ? m_MaxLoadSize : (static_cast<size_t>(fileSize) - offset);
    m_CurrentBounds[0] = offset;
    m_CurrentBounds[1] =  offset + loadSize; // this will store the offsets to the portion of the file currently loaded
    m_LoadedFileName = filepath;
    m_LoadedFileSize = fileSize;



    // Reserve space in the vector
    m_FileLoadData.clear(); // clear the old data out
    //m_FileLoadData.reserve(loadSize);
    // Read the entire file into the vector(char*)m_FileLoadData.data()

    utils::NewBuffer buffer(loadSize);
    file.read(reinterpret_cast<char*>(buffer.Get()), loadSize);
    auto tmp = std::vector<unsigned char>(buffer.Get(), buffer.Get() + loadSize);
    m_FileLoadData = tmp;
    file.close();
    return RETURN_CODE;
}

void FileBrowser::SetInputPath(const std::string& new_inputpath)
{
    memset(m_InputPath, 0x00, _MAX_PATH);
    memcpy(m_InputPath, new_inputpath.c_str(), sizeof(char) * new_inputpath.size());
}

