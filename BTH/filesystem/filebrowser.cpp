#include "filebrowser.h"

/*
    For some reason the Visual studio compiler was optimizing away this entire function such that it wasnt working
*/
#pragma optimize("", off) // Turn off optimizations
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
    if (RETURN_CODE == FB_RETCODE::FILE_CHANGE_LOAD)
    {
        m_Edits.clear();
    }
       
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
    // apply edits before return
    int edit_index = static_cast<int>(offset / m_MaxLoadSize);
    for (auto& edit : m_Edits[edit_index])
    {
        if (edit.m_Offset >= offset)
            tmp[edit.m_Offset - offset] = edit.m_ByteValue;       
    }


    m_FileLoadData = tmp;
    file.close();
    return RETURN_CODE;
}

std::vector<unsigned char> FileBrowser::LoadBytes(unsigned long long offset, DWORD numToRead, DWORD* numRead)
{
    auto& filename = m_LoadedFileName;
    utils::NewBuffer buffer(numToRead);
    std::ifstream file(filename, std::ios::binary);
    numToRead = (m_LoadedFileSize - offset > numToRead ? numToRead : m_LoadedFileSize - offset);
    if (!file.is_open())
        return {};
    file.seekg(offset, std::ios::beg);
    file.read(reinterpret_cast<char*>(buffer.Get()), numToRead);
    auto read = file.gcount();
    *numRead = read;
    auto ret = std::vector<unsigned char>(buffer.Get(), buffer.Get() + read);

    // apply edits before return
    int edit_index = static_cast<int>(offset / m_MaxLoadSize);
    for (auto& edit : m_Edits[edit_index])
    {
        if (edit.m_Offset >= offset)
            ret[edit.m_Offset - offset] = edit.m_ByteValue;
    }



    file.close();
    return ret;
}

void FileBrowser::SetInputPath(const std::string& new_inputpath)
{
    memset(m_InputPath, 0x00, _MAX_PATH);
    memcpy(m_InputPath, new_inputpath.c_str(), sizeof(char) * new_inputpath.size());
}


void FileBrowser::EditByte(int offset, unsigned char edit_value)
{
    if (offset < 0 || offset >= m_FileLoadData.size())
    {
        return;
    }

    int edit_key = static_cast<int>((m_CurrentBounds[0] + offset) / 200000);
    FileEdit edit_data = { edit_value, m_CurrentBounds[0] + offset };       // we give the absolute index into the file
    m_Edits[edit_key].push_back(edit_data);
    m_FileLoadData[offset] = edit_value;
}



bool FileBrowser::SaveFile(const std::string& save_path)
{
    fs::path savefile = fs::path(save_path);

    // Create the directories if they don't exist
    fs::create_directories(savefile.parent_path());

    // Open new file stream
    std::ofstream outputFile(savefile, std::ios::binary);

    DWORD read;
    size_t offset = 0;
    if (outputFile.is_open())
    {
        while (offset < m_LoadedFileSize)
        {
            auto FileBytes = LoadBytes(offset, m_MaxLoadSize, &read);
            auto relevant_edits = m_Edits[static_cast<int>(offset / 200000)];
            printf("edits %d\n", relevant_edits.size());
            for (auto& edit : relevant_edits)
            {
                FileBytes[edit.m_Offset] = edit.m_ByteValue;
            }


            // Use write to handle binary data
            outputFile.write(reinterpret_cast<char*>(FileBytes.data()), FileBytes.size());


            offset += read;
        }
        outputFile.close();
        if (offset != m_LoadedFileSize)
            return 0;
        return 1;
    }
    else
    {
        return 0;
    }
}

int FileBrowser::PushByte(unsigned char byte)
{
    if (m_FileLoadData.size() >= m_MaxLoadSize)
        return -1;

    m_FileLoadData.push_back(byte);
    return 0;
}
#pragma optimize("", on)  // Turn on optimizations
