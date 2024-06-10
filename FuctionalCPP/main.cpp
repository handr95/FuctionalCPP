// Standard library
#include <iostream>

// Our reactive stream implementation
#include "sink.h"

// Service implementation
#include "service.h"


int main(int argc, char* argv[])
{
    // io_service�� �̺�Ʈ ������ ó���ϴ� Boost.Asio�� Ŭ������. �̰��� �̺�Ʈ�� ������ �̺�Ʈ�� �´� ������ �ݹ� ���ٸ� ȣ���Ѵ�.
    boost::asio::io_service event_loop;

    // ���񽺸� ����� ��ũ�� �����Ѵ�.
    auto pipeline = sink(service(event_loop),
        [](const auto& message) {
            std::cerr << message << std::endl;
        });

    // �̺�Ʈ ó���� �����Ѵ�.
    event_loop.run();
}
