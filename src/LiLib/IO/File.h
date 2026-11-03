#pragma once
#include "LiLib/Containers/Array.h"
#include "LiLib/Core/Typedefs.h"
#include "LiLib/Strings/String.h"
#include "LiLib/Strings/StringView.h"

namespace LiLib {

/*
 * @brief Whole-file reads and writes, and simple queries. Paths are UTF-8 on every platform. Failures are reported
 *        through the return value and leave no partial result in the output.
 */
class File {
public:
    static bool ReadBytes(StringView path, Array<byte_t>& outBytes);
    static bool ReadText(StringView path, String& outText);

    /*
     * @brief Replaces the file's contents, creating it if needed. The parent folder must exist.
     */
    static bool WriteBytes(StringView path, const void* data, size_t size);
    static bool WriteText(StringView path, StringView text);

    static bool Exists(StringView path);

    /*
     * @brief Last-modified time in seconds since an unspecified, fixed point.
     * @returns false when the file does not exist.
     */
    static bool GetModifiedTime(StringView path, int64_t& outTime);

    /*
     * @brief Creates a folder and any missing parents. Succeeds if the folder already exists.
     */
    static bool CreateDirectories(StringView path);
};

} // namespace LiLib
