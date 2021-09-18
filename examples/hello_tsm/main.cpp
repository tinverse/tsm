#include <tsm.h>

using tsm::Event;
using tsm::State;

struct SocketSM : tsm::Hsm<SocketSM>
{
  public:
    SocketSM()
      : tsm::Hsm<SocketSM>()
    {
        IHsm::setStartState(&Closed);

        add(Closed, sock_open, Ready);
        add(Ready, connect, Open);
        add(Ready, bind, Bound);
        add(Bound, listen, Listening);
        add(Listening, accept, Listening);
        add(Listening, close, Closed);
        add(Open, close, Closed);
    }

    // Events
    Event sock_open, bind, listen, connect, accept, close;

    // States
    State Closed, Ready, Bound, Open, Listening;
};

using SocketHsm = tsm::SingleThreadedHsm<SocketSM>;

int
main()
{
    SocketHsm sm;

    // Important!!
    sm.startSM();

    // send events... and step the sm
    sm.sendEvent(sm.sock_open);
    sm.step();

    sm.stopSM();
}
