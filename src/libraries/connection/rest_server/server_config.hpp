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

#pragma once

#include <boost/asio/ip/tcp.hpp>

#include <cstddef>
#include <thread>


namespace connection::rest_server
{
    struct server_config
    {
        boost::asio::ip::address ip      = boost::asio::ip::make_address("127.0.0.1");
        unsigned short           port    = 8080;
        size_t                   threads = std::thread::hardware_concurrency();
    };
} // namespace connection::rest_server
