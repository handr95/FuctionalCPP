#pragma once

#ifndef SINK_H
#define SINK_H

#include <functional>

namespace detail {
	template < typename Sender
		, typename Function
		, typename MessageType = typename Sender::value_type
	>
	class sink_impl {
	public:
		using value_type = MessageType;

		sink_impl(Sender&& sender, Function function)
			: m_sender(std::move(sender))
			, m_function(function)
		{
			// 싱크가 생성되면 할당된 발신자에게 자동으로 연결된다.
			m_sender.on_message(
				[this](MessageType&& message)
				{
					process_message(std::move(message));
				}
			);
		}

		// 메시지를 받으면 사용자가 정의한 함수에 메시지를 전달한다.
		void process_message(MessageType&& message) const
		{
			std::invoke(m_function, std::move(message));
		}

	private:
		Sender m_sender;
		Function m_function;
	};

	template <typename Function>
	struct sink_helper {
		Function function;
	};

}

template <typename Sender, typename Function>
auto sink(Sender&& sender, Function&& function)
{
	return detail::sink_impl<Sender, Function>(
		std::forward<Sender>(sender),
		std::forward<Function>(function));
}

#endif

