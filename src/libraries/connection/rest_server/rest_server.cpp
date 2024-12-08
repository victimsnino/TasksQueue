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

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <rpp/rpp.hpp>

namespace asio  = boost::asio;
namespace beast = boost::beast;
namespace http  = beast::http;
using tcp       = asio::ip::tcp;

namespace connection::rest_server
{
    namespace
    {
        template<rpp::constraint::observer TObs>
        struct server_ctx final : public std::enable_shared_from_this<server_ctx<TObs>>
        {
            server_ctx(TObs&& observer, const server_config& config)
                : observer{std::move(observer)}
                , acceptor{ioc, tcp::endpoint{tcp::v4(), config.port}}
            {
            }

            void schedule_accept(bool first_schedule = true)
            {
                if (first_schedule)
                {
                    observer.set_upstream(rpp::make_callback_disposable([weak = this->weak_from_this()] noexcept {
                        if (const auto ctx = weak.lock())
                            ctx->ioc.stop();
                    }));
                }

                acceptor.async_accept([weak = this->weak_from_this()](beast::error_code ec, tcp::socket socket) {
                    if (const auto ctx = weak.lock())
                    {
                        if (!ec)
                        {
                            ctx->observer.on_next(std::move(socket));
                            ctx->schedule_accept(false);
                            return;
                        }
                        ctx->observer.on_error(std::make_exception_ptr(ec));
                    }
                });
            }

            TObs             observer;
            asio::io_context ioc;
            tcp::acceptor    acceptor;
        };
    } // namespace

    rpp::dynamic_observable<boost::asio::ip::tcp::socket> create(const server_config& config)
    {
        return rpp::source::create<boost::asio::ip::tcp::socket>([config]<rpp::constraint::observer TObs>(TObs&& observer) {
                   auto ctx = std::make_shared<server_ctx<std::decay_t<TObs>>>(std::forward<TObs>(observer), config);
                   ctx->schedule_accept();

                   for (size_t threads = 0; threads < std::max(size_t{1}, config.threads); ++threads)
                   {
                       std::thread{[ctx] {
                           ctx->ioc.run();
                       }}.detach();
                   }
               })
             | rpp::ops::publish()
             | rpp::ops::ref_count();
    }
} // namespace connection::rest_server
