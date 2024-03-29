/*

Copyright (c) 2003-2013, Daniel Wallin
Copyright (c) 2013, 2015-2020, Arvid Norberg
Copyright (c) 2016, 2018, 2020, Alden Torres
All rights reserved.

You may use, distribute and modify this code under the terms of the BSD license,
see LICENSE file.
*/

#ifndef TORRENT_ALERT_MANAGER_HPP_INCLUDED
#define TORRENT_ALERT_MANAGER_HPP_INCLUDED

#include "ip2/config.hpp"
#include "ip2/alert.hpp"
#include "ip2/aux_/heterogeneous_queue.hpp"
#include "ip2/aux_/stack_allocator.hpp"
#include "ip2/alert_types.hpp" // for abi_alert_count
#include "ip2/aux_/array.hpp"

#include <functional>
#include <utility> // for std::forward
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <bitset>

namespace ip2 {
namespace aux {

	struct TORRENT_EXTRA_EXPORT alert_manager
	{
		explicit alert_manager(int queue_limit
			, alert_category_t alert_mask = alert_category::error);

		alert_manager(alert_manager const&) = delete;
		alert_manager& operator=(alert_manager const&) = delete;

		~alert_manager();

		template <class T, typename... Args>
		void emplace_alert(Args&&... args) try
		{
			std::unique_lock<std::recursive_mutex> lock(m_mutex);

			heterogeneous_queue<alert>& queue = m_alerts[m_generation];

			// don't add more than this number of alerts, unless it's a
			// high priority alert, in which case we try harder to deliver it
			// for high priority alerts, double the upper limit
			if (queue.size() / (1 + static_cast<int>(T::priority)) >= m_queue_size_limit)
			{
				// record that we dropped an alert of this type
				m_dropped.set(T::alert_type);
				return;
			}

			T& alert = queue.emplace_back<T>(
				m_allocations[m_generation], std::forward<Args>(args)...);

			maybe_notify(&alert);
		}
		catch (std::bad_alloc const&)
		{
			// record that we dropped an alert of this type
			std::unique_lock<std::recursive_mutex> lock(m_mutex);
			m_dropped.set(T::alert_type);
		}

		bool pending() const;
		void get_all(std::vector<alert*>& alerts);

		template <class T>
		bool should_post() const
		{
			return bool(m_alert_mask.load(std::memory_order_relaxed) & T::static_category);
		}

		alert* wait_for_alert(time_duration max_wait);

		void set_alert_mask(alert_category_t const m) noexcept
		{
			m_alert_mask = m;
		}

		alert_category_t alert_mask() const noexcept
		{
			return m_alert_mask;
		}

		int alert_queue_size_limit() const noexcept { return m_queue_size_limit; }
		int set_alert_queue_size_limit(int queue_size_limit_);

		void set_notify_function(std::function<void()> const& fun);

	private:

		void maybe_notify(alert* a);

		// this mutex protects everything. Since it's held while executing user
		// callbacks (the notify function and extension on_alert()) it must be
		// recursive to support recursively post new alerts.
		mutable std::recursive_mutex m_mutex;
		std::condition_variable_any m_condition;
		std::atomic<alert_category_t> m_alert_mask;
		int m_queue_size_limit;

		// a bitfield where each bit represents an alert type. Every time we drop
		// an alert (because the queue is full or of some other error) we set the
		// corresponding bit in this mask, to communicate to the client that it
		// may have missed an update.
		std::bitset<abi_alert_count> m_dropped;

		// this function (if set) is called whenever the number of alerts in
		// the alert queue goes from 0 to 1. The client is expected to wake up
		// its main message loop for it to poll for alerts (using get_alerts()).
		// That call will drain every alert in one atomic operation and this
		// notification function will be called again the next time an alert is
		// posted to the queue
		std::function<void()> m_notify;

		// this is either 0 or 1, it indicates which m_alerts and m_allocations
		// the alert_manager is allowed to use right now. This is swapped when
		// the client calls get_all(), at which point all of the alert objects
		// passed to the client will be owned by ip2 again, and reset.
		int m_generation = 0;

		// this is where all alerts are queued up. There are two heterogeneous
		// queues to double buffer the thread access. The std::mutex in the alert
		// manager gives exclusive access to m_alerts[m_generation] and
		// m_allocations[m_generation] whereas the other copy is exclusively
		// used by the client thread.
		aux::array<heterogeneous_queue<alert>, 2> m_alerts;

		// this is a stack where alerts can allocate variable length content,
		// such as strings, to go with the alerts.
		aux::array<aux::stack_allocator, 2> m_allocations;

	};
}
}

#endif
