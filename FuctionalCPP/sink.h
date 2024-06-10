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
			// ��ũ�� �����Ǹ� �Ҵ�� �߽��ڿ��� �ڵ����� ����ȴ�.
			m_sender.on_message(
				[this](MessageType&& message)
				{
					process_message(std::move(message));
				}
			);
		}

		// �޽����� ������ ����ڰ� ������ �Լ��� �޽����� �����Ѵ�.
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

