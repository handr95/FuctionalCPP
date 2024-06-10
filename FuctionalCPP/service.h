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
        // �� ������ �������� �����ϴ� �ٸ� �����͸� �����.
        auto self = shared_session::shared_from_this();
        //�Է¿��� ���� ���ڿ� ������ �� ����� ���ٸ� ������ �Ѵ�.
        boost::asio::async_read_until(
            m_socket, m_data, '\n',
            [this, self](const boost::system::error_code& error,
                std::size_t size) {
                    if (!error) {
                        // �� ���� �о �޽����� �����ϱ� ���� ����� ��ü���� ����
                        std::istream is(&m_data);
                        std::string line;
                        std::getline(is, line);
                        m_emit(std::move(line));

                        // �޽����� ���������� �о��ٸ� ���� �޽����� �е��� �������Ѵ�.
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
    // Ŭ�󸮾�Ʈ�� �Է��� �� �پ� �д´�. ���� �޼������� ���ڿ��̴�.
    using value_type = std::string;

    // ������ ��Ʈ���� �����ϴ� ���񽺸� �����.(�⺻������ 42020).
    explicit service(boost::asio::io_service& service,
        unsigned short port = 42042)
        : m_acceptor(
            service,
            tcp::endpoint(tcp::v4(), port))
        , m_socket(service)
    {}

    // ���縦 ��Ȱ��ȭ������ �̵��� ����Ѵ�.
    service(const service& other) = delete;
    service(service&& other) = default;

    // �������� meesage_service�κ��� �޽����� �����ϴ� ���� ����� ������ Ŭ���̾�Ʈ�κ����� ������ �������� �ʴ� ����
    template <typename EmitFunction>
    void on_message(EmitFunction emit)
    {
        m_emit = emit;
        do_accept();
    }

private:
    // ������ Ŭ���̾�Ʈ�� ���� ������ ����� �����Ѵ�.
    // ���� ��ü�� Ŭ���̾�Ʈ���� �޽����� ������ m_emit���� ���޵ȴ�.
    // make_shard_sessoin�� ���� ��ü �ν��Ͻ��� ���� ���� �����͸� �����Ѵ�.
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