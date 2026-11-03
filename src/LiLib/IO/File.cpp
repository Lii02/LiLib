#include "LiLib/IO/File.h"
#include <sys/stat.h>
#include <cerrno>
#include <cstdio>
#ifdef _WIN32
#include <direct.h>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif

namespace LiLib {

namespace {

#ifdef _WIN32

Array<wchar_t> ToWide(StringView path) {
    const String terminated(path);
    const int count = MultiByteToWideChar(CP_UTF8, 0, terminated.CStr(), -1, nullptr, 0);
    Array<wchar_t> wide;
    wide.Resize(static_cast<size_t>(Max(count, 1)), L'\0');
    if(count > 0) {
        MultiByteToWideChar(CP_UTF8, 0, terminated.CStr(), -1, wide.Data(), count);
    }
    return wide;
}

std::FILE* OpenFile(StringView path, const wchar_t* mode) {
    std::FILE* file = nullptr;
    return _wfopen_s(&file, ToWide(path).Data(), mode) == 0 ? file : nullptr;
}

std::FILE* OpenForRead(StringView path) {
    return OpenFile(path, L"rb");
}

std::FILE* OpenForWrite(StringView path) {
    return OpenFile(path, L"wb");
}

bool MakeDirectory(StringView path) {
    return _wmkdir(ToWide(path).Data()) == 0 || errno == EEXIST;
}

bool ReadStat(StringView path, int64_t& outTime) {
    struct _stat64 info;
    if(_wstat64(ToWide(path).Data(), &info) != 0) {
        return false;
    }
    outTime = static_cast<int64_t>(info.st_mtime);
    return true;
}

#else

std::FILE* OpenForRead(StringView path) {
    return std::fopen(String(path).CStr(), "rb");
}

std::FILE* OpenForWrite(StringView path) {
    return std::fopen(String(path).CStr(), "wb");
}

bool MakeDirectory(StringView path) {
    return mkdir(String(path).CStr(), 0755) == 0 || errno == EEXIST;
}

bool ReadStat(StringView path, int64_t& outTime) {
    struct stat info;
    if(stat(String(path).CStr(), &info) != 0) {
        return false;
    }
    outTime = static_cast<int64_t>(info.st_mtime);
    return true;
}

#endif

// Reads everything left in an open file into data, which is resized to fit. Closes the file.
template<typename TOutput>
bool ReadAndClose(std::FILE* file, TOutput& data) {
    bool success = std::fseek(file, 0, SEEK_END) == 0;
    const long size = success ? std::ftell(file) : -1;
    success = size >= 0 && std::fseek(file, 0, SEEK_SET) == 0;
    if(success) {
        data.Resize(static_cast<size_t>(size));
        success = size == 0 || std::fread(data.Data(), 1, static_cast<size_t>(size), file) == static_cast<size_t>(size);
    }
    std::fclose(file);
    return success;
}

} // namespace

bool File::ReadBytes(StringView path, Array<byte_t>& outBytes) {
    std::FILE* file = OpenForRead(path);
    if(file == nullptr) {
        return false;
    }
    Array<byte_t> bytes;
    if(!ReadAndClose(file, bytes)) {
        return false;
    }
    outBytes = std::move(bytes);
    return true;
}

bool File::ReadText(StringView path, String& outText) {
    std::FILE* file = OpenForRead(path);
    if(file == nullptr) {
        return false;
    }
    String text;
    if(!ReadAndClose(file, text)) {
        return false;
    }
    outText = std::move(text);
    return true;
}

bool File::WriteBytes(StringView path, const void* data, size_t size) {
    std::FILE* file = OpenForWrite(path);
    if(file == nullptr) {
        return false;
    }
    const bool wrote = size == 0 || std::fwrite(data, 1, size, file) == size;
    const bool closed = std::fclose(file) == 0;
    return wrote && closed;
}

bool File::WriteText(StringView path, StringView text) {
    return WriteBytes(path, text.Data(), text.Length());
}

bool File::Exists(StringView path) {
    int64_t ignored = 0;
    return ReadStat(path, ignored);
}

bool File::GetModifiedTime(StringView path, int64_t& outTime) {
    return ReadStat(path, outTime);
}

bool File::CreateDirectories(StringView path) {
    for(size_t i = 1; i < path.Length(); ++i) {
        const char c = path[i];
        // Skips the separator after a drive letter, because "C:" itself can't be created
        if((c == '/' || c == '\\') && path[i - 1] != ':') {
            if(!MakeDirectory(path.Substr(0, i))) {
                return false;
            }
        }
    }
    return path.IsEmpty() || MakeDirectory(path);
}

} // namespace LiLib
