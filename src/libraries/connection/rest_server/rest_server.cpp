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
#include <rpp/operators/publish.hpp>
#include <rpp/operators/ref_count.hpp>
#include <rpp/sources/create.hpp>

namespace connection::rest_server
{
    namespace
    {
        template<rpp::constraint::observer TObs>
        class server_ctx final : public std::enable_shared_from_this<server_ctx<TObs>>
        {
        public:
            server_ctx(TObs&& observer, const server_config& config)
                : m_observer{std::move(observer)}
                , m_acceptor{m_ioc, boost::asio::ip::tcp::endpoint{boost::asio::ip::tcp::v4(), config.port}}
                , m_config{config}
            {
            }

            void run()
            {
                m_observer.set_upstream(rpp::make_callback_disposable([weak = this->weak_from_this()] noexcept {
                    if (const auto ctx = weak.lock())
                        ctx->m_ioc.stop();
                }));
                schedule_accept();

                for (size_t threads = 0; threads < std::max(size_t{1}, m_config.threads); ++threads)
                {
                    std::thread{[ctx = this->shared_from_this()] {
                        ctx->m_ioc.run();
                    }}.detach();
                }
            }

        private:
            void schedule_accept()
            {
                m_acceptor.async_accept([weak = this->weak_from_this()](boost::beast::error_code ec, boost::asio::ip::tcp::socket socket) {
                    if (const auto ctx = weak.lock())
                    {
                        if (!ec)
                        {
                            ctx->m_observer.on_next(std::move(socket));
                            ctx->schedule_accept();
                            return;
                        }
                        ctx->m_observer.on_error(std::make_exception_ptr(ec));
                    }
                });
            }

            TObs                           m_observer;
            boost::asio::io_context        m_ioc;
            boost::asio::ip::tcp::acceptor m_acceptor;
            server_config                  m_config;
        };
    } // namespace

    rpp::dynamic_observable<boost::asio::ip::tcp::socket> create(const server_config& config)
    {
        return rpp::source::create<boost::asio::ip::tcp::socket>([config]<rpp::constraint::observer TObs>(TObs&& observer) {
                   std::make_shared<server_ctx<std::decay_t<TObs>>>(std::forward<TObs>(observer), config)->run();
               })
             | rpp::ops::publish()
             | rpp::ops::ref_count();
    }
} // namespace connection::rest_server
