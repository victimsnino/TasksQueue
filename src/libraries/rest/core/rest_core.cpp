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

namespace rest
{
    std::string_view ParseContentType(rest::ContentType content_type)
    {
        switch (content_type)
        {
        case rest::ContentType::TextPlain: return "text/plain";
        case rest::ContentType::ApplicationJson: return "application/json";

        case rest::ContentType::Unknown:
        case rest::ContentType::MAX:
            return "";
        }
    }

    rest::ContentType ParseContentType(std::string_view content_type)
    {
        using EnumType = std::underlying_type_t<rest::ContentType>;
        for (auto type = static_cast<EnumType>(rest::ContentType::Unknown); type != static_cast<EnumType>(rest::ContentType::MAX); ++type)
            if (ParseContentType(static_cast<rest::ContentType>(type)) == content_type)
                return static_cast<rest::ContentType>(type);
        return rest::ContentType::Unknown;
    }
} // namespace rest
