#pragma once
#include "LiLib/Containers/Array.h"
#include "LiLib/Strings/String.h"

namespace LiLib {

/*
 * @brief Standard base64 encoding (RFC 4648).
 */
class Base64 {
public:
    /*
     * @brief Encodes a byte range as padded base64.
     */
    static String Encode(const void* data, size_t size);

    /*
     * @brief Decodes base64 into bytes. Whitespace is skipped and padding is optional.
     * @return False if the text contains invalid characters or has an invalid length; out is left empty.
     */
    static bool Decode(StringView text, Array<byte_t>& out);
};

} // namespace LiLib
