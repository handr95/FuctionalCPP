#ifndef SERVICE_H
#define SERVICE_H

// Standard library
#include <iostream>
#include <functional>

// Boost ASIO
#include <boost/asio.hpp>

// Not a good idea to add a using declaration in a header,
// but we can live with it for this small example
using boost::asio::ip::tcp;


/**
 * Session handling class.
 *
 * It reads the data sent by the client line by line,
 * and sends each line as a separate message.
 */
template <typename EmitFunction>
class session : public std::enable_shared_from_this<session<EmitFunction>> {
public:
    session(tcp::socket&& socket, EmitFunction emit)
        : m_socket(std::move(socket))
        , m_emit(emit)
    {
    }

    void start()
    {
        do_read();
    }

private:
    using shared_session = std::enable_shared_from_this<session<EmitFunction>>;

    void do_read()
    {
        // 이 세션의 소유권을 공유하는 다른 포인터를 만든다.
        auto self = shared_session::shared_from_this();
        //입력에서 개행 문자에 도달할 떄 실행될 람다를 스케쥴 한다.
        boost::asio::async_read_until(
            m_socket, m_data, '\n',
            [this, self](const boost::system::error_code& error,
                std::size_t size) {
                    if (!error) {
                        // 한 줄을 읽어서 메시지를 리슨하기 위해 등록한 주체에게 보냄
                        std::istream is(&m_data);
                        std::string line;
                        std::getline(is, line);
                        m_emit(std::move(line));

                        // 메시지를 성공적으로 읽었다면 다음 메시지를 읽도록 스케쥴한다.
                        do_read();
                    }
            });
    }

    tcp::socket m_socket;
    boost::asio::streambuf m_data;
    EmitFunction m_emit;
};

/**
 * Constructs a shared pointer to the session specified
 * by the socket and sets the function which the session will
 * used for sending the messages
 */
template <typename Socket, typename EmitFunction>
auto make_shared_session(Socket&& socket, EmitFunction&& emit)
{
    return std::make_shared<session<EmitFunction>>(
        std::forward<Socket>(socket),
        std::forward<EmitFunction>(emit));
}

/**
 * The service class handles client connections
 * and emits the messages sent by the clients
 */
class service {
public:
    // 클라리언트의 입력을 한 줄씩 읽는다. 보낸 메세시지는 문자열이다.
    using value_type = std::string;

    // 지정된 포트에서 리슨하는 서비스를 만든다.(기본적으로 42020).
    explicit service(boost::asio::io_service& service,
        unsigned short port = 42042)
        : m_acceptor(
            service,
            tcp::endpoint(tcp::v4(), port))
        , m_socket(service)
    {}

    // 복사를 비활성화하지만 이동은 허용한다.
    service(const service& other) = delete;
    service(service&& other) = default;

    // 누군가가 meesage_service로부터 메시지를 리슨하는 것을 등록할 때까지 클라이언트로부터의 연결을 수락하지 않는 지점
    template <typename EmitFunction>
    void on_message(EmitFunction emit)
    {
        m_emit = emit;
        do_accept();
    }

private:
    // 들어오는 클라이언트에 대한 세션을 만들고 시작한다.
    // 세션 객체가 클라이언트에서 메시지를 읽으면 m_emit으로 전달된다.
    // make_shard_sessoin은 세션 객체 인스턴스에 대한 공유 포인터를 생성한다.
    void do_accept();
    tcp::acceptor m_acceptor;
    tcp::socket m_socket;
    std::function<void(std::string&&)> m_emit;


    friend std::ostream& operator<< (std::ostream& out, const service& service)
    {
        return out << "service object";
    }
};

#endif