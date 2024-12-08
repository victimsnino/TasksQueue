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

//
// Copyright (c) 2022 Klemens D. Morgenstern (klemens dot morgenstern at gmx dot net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

//------------------------------------------------------------------------------
//
// Example: HTTP server, coroutine
//
//------------------------------------------------------------------------------

// // #include <boost/asio/awaitable.hpp>
// #include <boost/asio/co_spawn.hpp>
// #include <boost/asio/io_context.hpp>
// #include <boost/asio/ip/tcp.hpp>
// #include <boost/beast/core.hpp>
// #include <boost/beast/http.hpp>
// #include <boost/beast/version.hpp>
// #include <boost/config.hpp>

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#if defined(BOOST_ASIO_HAS_CO_AWAIT)

namespace beast = boost::beast;
namespace http  = beast::http;
namespace net   = boost::asio;


// Handles an HTTP server connection
net::awaitable<void>
do_session(
    beast::tcp_stream                  stream,
    std::shared_ptr<std::string const> doc_root)
{
    // This buffer is required to persist across reads
    beast::flat_buffer buffer;

    // for(;;)
    {
        // Set the timeout.
        stream.expires_after(std::chrono::seconds(30));

        // Read a request
        http::request<http::string_body> req;
        co_await http::async_read(stream, buffer, req);

        for (size_t c = 0; stream.socket().is_open(); ++c)
        {
            http::response<http::string_body> res{http::status::ok, req.version()};
            res.set(http::field::content_type, "text/event-stream");
            res.keep_alive(true);
            res.body() = std::to_string(c);


            // Send the response
            try
            {
                co_await beast::async_write(stream, http::message_generator{std::move(res)});
            }
            catch (std::exception const & e)
            {
                std::cerr << ">>> Error in session: " << e.what() << "\n";
            }
        }
    }

    // Send a TCP shutdown
    stream.socket().shutdown(net::ip::tcp::socket::shutdown_send);

    // At this point the connection is closed gracefully
    // we ignore the error because the client might have
    // dropped the connection already.
}

// Accepts incoming connections and launches the sessions
net::awaitable<void>
do_listen(net::ip::tcp::endpoint endpoint, std::shared_ptr<std::string const> doc_root)
{
    auto executor = co_await net::this_coro::executor;
    auto acceptor = net::ip::tcp::acceptor{executor, endpoint};

    for (;;)
    {
        net::co_spawn(
            executor,
            do_session(
                beast::tcp_stream{co_await acceptor.async_accept()},
                doc_root),
            [](std::exception_ptr e) {
                if (e)
                {
                    try
                    {
                        std::rethrow_exception(e);
                    }
                    catch (std::exception const & e)
                    {
                        std::cerr << "Error in session: " << e.what() << "\n";
                    }
                }
            });
    }
}

int main(int argc, char* argv[])
{
    // Check command line arguments.
    if (argc != 5)
    {
        std::cerr << "Usage: http-server-awaitable <address> <port> <doc_root> <threads>\n"
                  << "Example:\n"
                  << "    http-server-awaitable 0.0.0.0 8080 . 1\n";
        return EXIT_FAILURE;
    }
    auto const address  = net::ip::make_address(argv[1]);
    auto const port     = static_cast<unsigned short>(std::atoi(argv[2]));
    auto const doc_root = std::make_shared<std::string>(argv[3]);
    auto const threads  = std::max<int>(1, std::atoi(argv[4]));

    // The io_context is required for all I/O
    net::io_context ioc{threads};

    // Spawn a listening port
    net::co_spawn(
        ioc,
        do_listen(net::ip::tcp::endpoint{address, port}, doc_root),
        [](std::exception_ptr e) {
            if (e)
            {
                try
                {
                    std::rethrow_exception(e);
                }
                catch (std::exception const & e)
                {
                    std::cerr << "Error: " << e.what() << std::endl;
                }
            }
        });

    ioc.run();

    return EXIT_SUCCESS;
}

#else

int main(int, char*[])
{
    std::printf("awaitables require C++20\n");
    return EXIT_FAILURE;
}

#endif
