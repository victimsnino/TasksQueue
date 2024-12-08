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

#include <doctest/doctest.h>
#include <doctest/trompeloeil.hpp>

namespace test_utils
{
    template<typename T>
    class mock_observer
    {
    public:
        static constexpr auto preferred_disposables_mode = rpp::details::observers::disposables_mode::Auto;

        struct impl_t
        {
            impl_t() = default;

            MAKE_MOCK1(on_next, void(const T&), const);
            MAKE_MOCK1(on_error, void(const std::exception_ptr& err), const);
            MAKE_MOCK0(on_completed, void(), const);
        };

        impl_t& operator*() const noexcept { return *m_impl; }

        void on_next(const T& v) const
        {
            m_impl->on_next(v);
        }

        void on_error(const std::exception_ptr& err) const noexcept { m_impl->on_error(err); }
        void on_completed() const noexcept { m_impl->on_completed(); }

        static bool is_disposed() noexcept { return false; }
        static void set_upstream(const rpp::disposable_wrapper&) noexcept {}

        auto get_observer() const { return rpp::observer<T, mock_observer<T>>{*this}; }
        auto get_observer(rpp::composite_disposable_wrapper d) const { return rpp::observer_with_external_disposable<T, mock_observer<T>>{std::move(d), *this}; }

    private:
        std::shared_ptr<impl_t> m_impl = std::make_shared<impl_t>();
    };

    template<typename T>
    inline void wait(const std::unique_ptr<T>& e)
    {
        while (!e->is_satisfied())
        {
            std::this_thread::sleep_for(std::chrono::seconds{1});
        }
    }
} // namespace test_utils
