#include "tsm.h"
using namespace tsm;

namespace tsmtest {

struct BHsm : public Hsm<BHsm>
{
    BHsm()
      : Hsm<BHsm>("BHsm")
      , s1("BS1")
    {
        add(s1, e1, s1);
    }

    State* getStartState() { return &s1; }
    State* getStopState() { return nullptr; }

    // States
    State s1;

    // Events
    Event e1;
};

struct AHsm : public Hsm<AHsm>
{
    AHsm()
      : Hsm<AHsm>("AHsm")
      , s1("s1")
      , s2("s2")
      , s3("s3")
      , s4("s4")
    {
        // Establish Parent - Child Relationships
        bHsm.setParent(this);

        // Create State Transition Table
        add(s1, e1, s2);
        add(s2, e2_in, bHsm);
        add(bHsm, e2_out, s3);
        add(s3, e3, s1);
        add(s3, end_event, s4);
    }

    State* getStartState() { return &s1; }
    State* getStopState() { return &s4; }

    // States
    BHsm bHsm;
    State s1;
    State s2;
    State s3;
    State s4;

    // Event
    Event e1;
    Event e2_in;
    Event e2_out;
    Event e3;
    Event end_event;
};
} // namespace tsmtest
