// Standard library
#include <iostream>

// Our reactive stream implementation
#include "sink.h"

// Service implementation
#include "service.h"


int main(int argc, char* argv[])
{
    // io_service는 이벤트 루프를 처리하는 Boost.Asio의 클래스다. 이것은 이벤트를 리슨해 이벤트에 맞는 적절한 콜백 람다를 호출한다.
    boost::asio::io_service event_loop;

    // 서비스를 만들어 싱크에 연결한다.
    auto pipeline = sink(service(event_loop),
        [](const auto& message) {
            std::cerr << message << std::endl;
        });

    // 이벤트 처리를 시작한다.
    event_loop.run();
}
