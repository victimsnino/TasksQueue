// Copyright (C) 2024-2024 Aleksey Loginov
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//
// Home page: https://github.com/victimsnino/TasksQueue/

#include "rest_core.hpp"

#include <libraries/utils/utils.hpp>
#include <rfl/enums.hpp>

namespace rest
{
    /**
     * Converts a ContentType enum to its corresponding MIME type string representation.
     * @param content_type The content type enum to convert
     * @return A string_view containing the MIME type string
     * @throws Assertion error if an invalid/unhandled content type is provided
     */
    std::string_view ParseContentType(rest::ContentType content_type)
    {
        switch (content_type)
        {
        case rest::ContentType::TextPlain: return "text/plain";
        case rest::ContentType::ApplicationJson: return "application/json";
        }
        ENSURE_MSG(false, "Invalid content type");
    }

    /**
     * Parses a string representation of content type and returns the corresponding ContentType enum value.
     * @param content_type The string view containing the content type to parse
     * @return An optional containing the matched ContentType enum value, or empty optional if no match is found
     */
    std::optional<rest::ContentType> ParseContentType(std::string_view content_type)
    {
        for (auto [_, type] : rfl::get_enumerator_array<rest::ContentType>())
            if (ParseContentType(type) == content_type)
                return type;
        return {};
    }
} // namespace rest
