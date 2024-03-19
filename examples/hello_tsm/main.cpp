#include <tsm.h>

// Example TCP socket state machine
namespace tcp_socket {
    Socket {
        // Events - sock_open, bind, listen, connect, accept, close;
        struct sock_open {};
        struct bind {};
        struct listen {};
        struct connect {};
        struct accept {};
        struct close {};

        // States - Closed, Ready, Bound, Open, Listening;
        struct Closed {};
        struct Ready {};
        struct Bound {};
        struct Open {};
        struct Listening {};

        using transitions = std::tuple<Transition<Closed, sock_open, Ready>,
                                       Transition<Ready, bind, Bound>,
                                       Transition<Bound, listen, Listening>,
                                       Transition<Ready, connect, Open>,
                                       Transition<Open, close, Closed>,
                                       Transition<Bound, close, Closed>,
                                       Transition<Listening, close, Closed>,
                                       Transition<Open, accept, Open>>;
        // ip address, port, backlog
        std::string ip;
        int port;
        int backlog;
    };
} // namespace tcp_socket


using SocketHsm = tsm::SingleThreadedHsm<tcp_socket::Socket>;

int
main()
{
    SocketHsm sm;
    // send events... and step the sm
    sm.sendEvent(tcp_socket::sock_open{});
    // Do other stuff and then step the sm
    sm.step();
}
