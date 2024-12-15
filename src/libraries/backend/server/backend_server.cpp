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

#include "backend_server.hpp"

#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/beast.hpp>
#include <boost/regex.hpp>

#include <iostream>
#include <thread>

namespace beast = boost::beast;
namespace http  = beast::http;
namespace net   = boost::asio;
using tcp       = boost::asio::ip::tcp;

namespace backend
{
    namespace
    {
        class Router
        {
        public:
            using Params = std::unordered_map<std::string, std::string>;
            using Request = http::request<http::string_body>;
            using Response = http::response<http::string_body>;
            using Handler = std::function<Response(const Request&, const Params&)>;

        private:
            struct Route
            {
                boost::regex                            pattern;         // Regex pattern for the route
                std::vector<std::string>                parameter_names; // List of parameter names
                std::unordered_map<http::verb, Handler> handlers;        // Handlers mapped by HTTP method
            };

            std::vector<Route> m_routes;

        public:
            void AddRoute(const std::string& path, http::verb method, Handler handler)
            {
                // Replace {:name} with regex group `([^/]+)` and extract parameter names
                boost::regex param_regex(R"(\{\:([a-zA-Z_][a-zA-Z0-9_]*)\})");
                std::string  regex_path = boost::regex_replace(path, param_regex, "([^/]+)");
                boost::regex full_regex("^" + regex_path + "$"); // Ensure full match

                std::vector<std::string> parameter_names;
                auto                     begin = boost::sregex_iterator(path.begin(), path.end(), param_regex);
                auto                     end   = boost::sregex_iterator();
                for (auto it = begin; it != end; ++it)
                {
                    parameter_names.push_back((*it)[1]);
                }

                // Store the route
                m_routes.push_back({full_regex, parameter_names, {{method, std::move(handler)}}});
            }

            std::optional<Response> Route(const Request& req) const
            {
                for (const auto& route : m_routes)
                {
                    boost::smatch match;
                    std::string   target = req.target();
                    if (boost::regex_match(target, match, route.pattern))
                    {
                        // Extract parameter values
                        Params params;
                        for (size_t i = 0; i < route.parameter_names.size(); ++i)
                        {
                            params[route.parameter_names[i]] = match[i + 1]; // First group is at index 1
                        }

                        // Find and call the handler
                        auto handler_it = route.handlers.find(req.method());
                        if (handler_it != route.handlers.end())
                        {
                            return handler_it->second(req, params);
                        }
                        else
                        {
                            return Response{http::status::method_not_allowed, req.version()};
                        }
                    }
                }
                return std::nullopt; // No matching route found
            }
        };

        struct ServerContext
        {
            ServerContext() = default;

            Router           router;
            std::atomic_bool started{};
        };

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

        net::awaitable<void> DoSession(beast::tcp_stream stream, std::shared_ptr<ServerContext> ctx)
        {
            // This buffer is required to persist across reads
            beast::flat_buffer buffer;

            stream.expires_after(std::chrono::seconds(30));

            http::request<http::string_body> req;
            co_await http::async_read(stream, buffer, req);

            co_await beast::async_write(stream, boost::beast::http::message_generator{ctx->router.Route(req).value_or(http::response<http::string_body>{http::status::not_found, req.version()})});

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

        boost::asio::io_context   ioc;
        std::vector<std::jthread> threads{};
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

    StopHandler StartServer(const TasksManager& manager, const ServerConfig& config)
    {
        auto       server_ctx      = std::make_shared<ServerContext>();
        server_ctx->router.AddRoute("/tasks", http::verb::get, [manager](const Router::Request& req, const Router::Params&){
            manager.GetTasks();
            return http::response<http::string_body>{http::status::ok, req.version()};
        });
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
} // namespace backend
