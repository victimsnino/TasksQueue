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
        /**
         * Logs an exception to standard error stream.
         * @param e Exception pointer to be logged. If the pointer is valid,
         *          the exception will be rethrown and caught to extract its message.
         * @note This function writes to std::cerr and does not throw exceptions.
         */
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
            /**
             * Constructs a ServerContext with a router.
             * @param router The router to be moved into the ServerContext.
             */
            explicit ServerContext(Router&& router)
                : router(std::move(router))
            {
            }

            Router           router;
            std::atomic_bool started{};
        };

        /**
         * Converts an HTTP verb to a REST request method.
         * @param method The HTTP verb to convert
         * @return An optional containing the corresponding REST request method, or empty if the verb is not supported
         */
        std::optional<rest::Request::Method> ParseMethod(http::verb method)
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
            default: return {};
            }
        }

        /**
         * Creates a Beast HTTP response from a REST response object.
         * @param response The REST response object containing status code, content type and body
         * @return A fully prepared Beast HTTP response with string body, server header set to "TasksQueue"
         *         and appropriate content type
         * @note The response payload is automatically prepared before returning
         */
        boost::beast::http::response<boost::beast::http::string_body> CreateResponse(const rest::Response& response)
        {
            boost::beast::http::response<boost::beast::http::string_body> res;
            res.result(static_cast<uint16_t>(response.status_code.get()));
            res.set(http::field::server, "TasksQueue");
            res.set(http::field::content_type, ParseContentType(response.content_type));
            res.body() = response.body;
            res.prepare_payload();
            return res;
        }

        /**
         * Prepares an HTTP response by validating and routing an incoming HTTP request.
         *
         * @param req The HTTP request to process, containing method, headers, and body
         * @param router The router object responsible for handling the request routing
         *
         * @return Response object containing:
         *         - On success: The routed response from the handler
         *         - On error: A 405 status for unsupported methods
         *                     A 400 status for invalid content type or accept headers
         *
         * @throws None
         *
         * The function validates:
         * - HTTP method
         * - Content-Type header
         * - Accept header
         *
         * If any validation fails, returns an error response with appropriate status code
         * and plain text error message.
         */
        Response PrepareResponse(const http::request<http::string_body>& req, const Router& router)
        {
            const auto method = ParseMethod(req.method());
            if (!method)
                return Response{.status_code = Response::Status::MethodNotAllowed, .body = "Unsupported or unknown method", .content_type = rest::ContentType::TextPlain};

            const auto content_type = ParseContentType(req[http::field::content_type]);
            if (!content_type)
                return Response{.status_code = Response::Status::BadRequest, .body = "Unsupported or unknown content type", .content_type = rest::ContentType::TextPlain};

            const auto accept_content_type = ParseContentType(req[http::field::accept]);
            if (!accept_content_type)
                return Response{.status_code = Response::Status::BadRequest, .body = "Unsupported or unknown accept content type", .content_type = rest::ContentType::TextPlain};

            return router.Route({.method = method.value(), .path = req.target(), .body = req.body(), .content_type = content_type.value(), .accept_content_type = accept_content_type.value()});
        }

        /**
         * Handles a single HTTP session asynchronously using Boost.Beast.
         *
         * @param stream TCP stream for the connection
         * @param ctx Shared server context containing routing information
         *
         * @details
         * The function performs the following operations:
         * 1. Sets a 30-second timeout for the stream
         * 2. Reads an HTTP request asynchronously
         * 3. Processes the request through the router
         * 4. Sends back the response
         * 5. Performs a clean TCP shutdown
         *
         * @throws boost::system::system_error On network or protocol errors
         * @throws std::runtime_error On session handling failures
         *
         * @note This is a coroutine that returns a net::awaitable<void>
         */
        net::awaitable<void> DoSession(beast::tcp_stream stream, std::shared_ptr<ServerContext> ctx)
        {
            // This buffer is required to persist across reads
            beast::flat_buffer buffer;

            stream.expires_after(std::chrono::seconds(30));

            http::request<http::string_body> req;
            co_await http::async_read(stream, buffer, req);

            co_await beast::async_write(stream, boost::beast::http::message_generator{CreateResponse(PrepareResponse(req, ctx->router))});

            // Send a TCP shutdown
            stream.socket().shutdown(net::ip::tcp::socket::shutdown_send);
        }

        /**
         * Asynchronously listens for incoming TCP connections and spawns session handlers.
         *
         * @param endpoint The TCP endpoint to listen on
         * @param ctx Shared server context containing server state and configuration
         * @return An awaitable that never completes normally (runs until server shutdown)
         * 
         * @details
         * This coroutine sets up a TCP acceptor that:
         * - Opens the specified endpoint
         * - Enables address reuse
         * - Binds to the specified endpoint
         * - Listens for incoming connections
         * - Notifies server startup through the context
         * - Continuously accepts new connections and spawns session handlers
         *
         * @throws boost::system::system_error On socket/binding/listening errors
         * @throws std::bad_alloc If memory allocation fails
         *
         * @note This function runs indefinitely in an accept loop until the server is shut down
         * @note Thread-safe: Multiple instances can run on different endpoints
         */
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
        /**
         * Constructs a ServerLifetime instance with a specified number of threads.
         * @param threads The number of threads to allocate for the IO context.
         */
        explicit ServerLifetime(int threads)
            : ioc{threads}
        {
        }

        boost::asio::io_context  ioc;
        std::vector<std::thread> threads{};
    };

    /**
     * Constructor for the StopHandler class.
     * @param ctx A shared pointer to ServerLifetime object that manages the server's lifecycle.
     *           Ownership of the pointer is transferred to this handler.
     */
    StopHandler::StopHandler(std::shared_ptr<ServerLifetime> ctx)
        : m_ctx{std::move(ctx)}
    {
    }

    /**
     * Destructor for the StopHandler class.
     * Ensures cleanup by calling Stop() before destruction.
     * @note This destructor is marked noexcept and will not throw exceptions.
     */
    StopHandler::~StopHandler() noexcept
    {
        Stop();
    }

    /**
     * Stops the IO context and joins all associated threads.
     * 
     * This method safely stops the IO context if it's running and ensures all threads
     * are properly joined. It is thread-safe and idempotent - multiple calls will not
     * cause issues.
     * 
     * @note This is a blocking call that waits for all threads to complete
     * @thread_safety Thread-safe
     */
    void StopHandler::Stop() const
    {
        if (!m_ctx->ioc.stopped())
        {
            m_ctx->ioc.stop();
            for (auto& t : m_ctx->threads)
                if (t.joinable())
                    t.join();
        }
    }

    /**
     * Starts an asynchronous HTTP server with the specified router and configuration.
     *
     * @param router Router object containing route handlers (moved into server context)
     * @param config Server configuration containing address, port, and thread settings
     *
     * @return StopHandler object that can be used to gracefully shutdown the server
     *
     * @details
     * The server is started with the following sequence:
     * 1. Creates a server context with the provided router
     * 2. Initializes a thread pool based on configuration
     * 3. Spawns an asynchronous listener on the specified endpoint
     * 4. Starts worker threads to handle incoming connections
     * 5. Waits for server to fully start before returning
     *
     * @note The server runs asynchronously in the background after this function returns
     * @thread_safety Thread-safe after server initialization
     */
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
