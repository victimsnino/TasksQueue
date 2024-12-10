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

#include <doctest/doctest.h>
#include <doctest/trompeloeil.hpp>

#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/http.hpp>
#include <libraries/connection/rest_server/rest_server.hpp>
#include <libraries/test_utils/test_utils.hpp>

TEST_CASE("rest_server provides observable of sockets")
{
    trompeloeil::sequence                                   s{};
    test_utils::mock_observer<boost::asio::ip::tcp::socket> mock{};

    boost::asio::io_context io_context;

    std::thread{[&io_context] {
        io_context.run();
    }}.detach();

    const auto d = connection::rest_server::create(connection::rest_server::server_config{.threads = 1}).subscribe_with_disposable(mock);

    const auto connect_socket = [&] {
        boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::make_address("127.0.0.1"), 8080);
        boost::asio::ip::tcp::socket   socket(io_context);
        socket.connect(endpoint);
        return socket;
    };

    SUBCASE("observable handles 1 socket")
    {
        const auto r = NAMED_REQUIRE_CALL(*mock, on_next(trompeloeil::_)).IN_SEQUENCE(s);
        connect_socket();
        test_utils::wait(r);
    }
    SUBCASE("observable handles 10 sockets")
    {
        for (size_t i = 0; i < 10; ++i)
        {
            const auto r = NAMED_REQUIRE_CALL(*mock, on_next(trompeloeil::_)).IN_SEQUENCE(s);
            connect_socket();
            test_utils::wait(r);
        }
    }

    d.dispose();
}
