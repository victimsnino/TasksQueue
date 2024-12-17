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

#include "rest_server.hpp"

#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/http.hpp>

#include <iostream>
#include <thread>

namespace beast = boost::beast;
namespace http  = beast::http;
namespace net   = boost::asio;
using tcp       = boost::asio::ip::tcp;

namespace rest
{
    namespace
    {
        void LogError(std::exception_ptr e)
        {
            if (e)
            {
                try
                {
                    std::rethrow_exception(e);
                }
                catch (std::exception& e)
                {
                    std::cerr << "Error in session: " << e.what() << "\n";
                }
            }
        };

        struct ServerContext
        {
            ServerContext(Router&& router)
                : router(std::move(router))
            {
            }

            Router           router;
            std::atomic_bool started{};
        };

        rest::Request::Method ParseMethod(http::verb method)
        {
            switch (method)
            {
            case http::verb::get: return rest::Request::Method::Get;
            case http::verb::post: return rest::Request::Method::Post;
            case http::verb::put: return rest::Request::Method::Put;
            case http::verb::delete_: return rest::Request::Method::Delete;
            case http::verb::patch: return rest::Request::Method::Patch;
            case http::verb::head: return rest::Request::Method::Head;
            case http::verb::options: return rest::Request::Method::Options;
            default: return rest::Request::Method::Unknown;
            }
        }

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

        boost::beast::http::response<boost::beast::http::string_body> CreateResponse(const rest::Response& response)
        {
            boost::beast::http::response<boost::beast::http::string_body> res;
            res.result(static_cast<uint16_t>(response.status_code));
            res.set(http::field::server, "TasksQueue");
            res.set(http::field::content_type, ParseContentType(response.content_type));
            res.body() = response.body;
            res.prepare_payload();
            return res;
        }

        net::awaitable<void> DoSession(beast::tcp_stream stream, std::shared_ptr<ServerContext> ctx)
        {
            // This buffer is required to persist across reads
            beast::flat_buffer buffer;

            stream.expires_after(std::chrono::seconds(30));

            http::request<http::string_body> req;
            co_await http::async_read(stream, buffer, req);

            co_await beast::async_write(stream, boost::beast::http::message_generator{CreateResponse(ctx->router.Route({.method = ParseMethod(req.method()), .path = req.target(), .body = req.body(), .content_type = ParseContentType(req[http::field::content_type])}))});

            // Send a TCP shutdown
            stream.socket().shutdown(net::ip::tcp::socket::shutdown_send);
        }

        net::awaitable<void> DoListen(net::ip::tcp::endpoint endpoint, std::shared_ptr<ServerContext> ctx)
        {
            auto acceptor = net::use_awaitable.as_default_on(tcp::acceptor(co_await net::this_coro::executor));
            acceptor.open(endpoint.protocol());

            // Allow address reuse
            acceptor.set_option(net::socket_base::reuse_address(true));

            // Bind to the server address
            acceptor.bind(endpoint);

            // Start listening for connections
            acceptor.listen(net::socket_base::max_listen_connections);

            ctx->started.store(true);
            ctx->started.notify_all();

            for (;;)
            {
                boost::asio::co_spawn(
                    acceptor.get_executor(),
                    DoSession(boost::beast::tcp_stream(co_await acceptor.async_accept()), ctx),
                    &LogError);
            }
        }
    } // namespace

    struct ServerLifetime
    {
        explicit ServerLifetime(int threads)
            : ioc{threads}
        {
        }

        boost::asio::io_context  ioc;
        std::vector<std::thread> threads{};
    };

    StopHandler::StopHandler(std::shared_ptr<ServerLifetime>&& ctx)
        : m_ctx{std::move(ctx)}
    {
    }

    StopHandler::~StopHandler() noexcept
    {
        Stop();
    }

    void StopHandler::Stop()
    {
        if (!m_ctx->ioc.stopped())
        {
            m_ctx->ioc.stop();
            for (auto& t : m_ctx->threads)
                if (t.joinable())
                    t.join();
        }
    }

    StopHandler StartServer(Router&& router, const ServerConfig& config)
    {
        auto server_ctx = std::make_shared<ServerContext>(std::move(router));

        const auto max_threads     = std::max(size_t{1}, config.threads);
        auto       server_lifetime = std::make_shared<ServerLifetime>(max_threads);

        net::co_spawn(server_lifetime->ioc, DoListen(net::ip::tcp::endpoint{net::ip::make_address(config.address), config.port}, server_ctx), &LogError);

        for (size_t threads = 0; threads < max_threads; ++threads)
        {
            server_lifetime->threads.emplace_back([server_lifetime] {
                server_lifetime->ioc.run();
            });
        }

        server_ctx->started.wait(false);

        return StopHandler{std::move(server_lifetime)};
    }
} // namespace rest
