#include "tsm.h"

namespace tsmtest {
using tsm::Event;
using tsm::Hsm;
using tsm::OrthogonalHsm;
using tsm::State;

struct BHsm : public Hsm<BHsm>
{
    BHsm()
    {
        IHsm::setStartState(&s1);

        add(s1, e1, s1);
    }

    // States
    State s1;

    // Events
    Event e1;
};

struct AHsm : public Hsm<AHsm>
{
    AHsm()
    {

        IHsm::setStartState(&s1);
        IHsm::setStopState(&s4);

        // Establish Parent - Child Relationships
        bHsm.setParent(this);

        // Create State Transition Table
        add(s1, e1, s2);
        add(s2, e2_in, bHsm);
        add(bHsm, e2_out, s3);
        add(s3, e3, s1);
        add(s3, end_event, s4);
    }

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

struct CHsm : public Hsm<CHsm>
{
    CHsm()
    {
        IHsm::setStartState(&s1);
        IHsm::setStopState(&s4);

        // Establish Parent - Child Relationships
        bHsm.setParent(this);

        // Create State Transition Table
        add(s1, e1, s2);
        add(s2, e2_in, bHsm);
        add(bHsm, e2_out, s3);
        add(s3, e3, s1);
        add(s3, end_event, s4);
    }

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

using OHsm = OrthogonalHsm<AHsm, CHsm>;

struct DHsm : Hsm<DHsm>
{
    DHsm()
    {
        IHsm::setStartState(&ds1);
        IHsm::setStopState(&ds5);

        // Establish Parent - Child Relationships
        bHsm.setParent(this);
        oHsm.setParent(this);

        // Create State Transition Table
        add(ds1, e1, ds2);
        add(ds2, e2_in, bHsm);
        add(bHsm, e2_out, ds3);
        add(ds3, e3, ds1);
        add(ds3, o_in, oHsm);
        add(oHsm, o_out, ds4);
        add(ds4, e4, ds3);
        add(ds4, end, ds5);
        add(ds3, end, ds5);
    }

    // States
    BHsm bHsm;
    OHsm oHsm;
    State ds1;
    State ds2;
    State ds3;
    State ds4;
    State ds5;

    // Event
    Event e1;
    Event e2_in;
    Event e2_out;
    Event e3;
    Event e4;
    Event o_in;
    Event o_out;
    Event end;
};
} // namespace tsmtest
