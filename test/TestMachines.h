#include "tsm.h"

namespace tsmtest {
using tsm::Event;
using tsm::Hsm;
using tsm::OrthogonalHsm;
using tsm::State;

struct BHsm : public Hsm<BHsm>
{
    BHsm()
      : Hsm<BHsm>("BHsm")
      , s1("BS1")
    {
        add(s1, e1, s1);
    }

    State* getStartState() override { return &s1; }
    State* getStopState() override { return nullptr; }

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

struct CHsm : public Hsm<CHsm>
{
    CHsm()
      : Hsm<CHsm>("CHsm")
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

struct OHsm : OrthogonalHsm<AHsm, CHsm>
{

    OHsm()
      : OrthogonalHsm<AHsm, CHsm>("OHsm")
    {}
};

struct DHsm : Hsm<DHsm>
{
    DHsm()
      : Hsm<DHsm>("DHsm")
      , ds1("ds1")
      , ds2("ds2")
      , ds3("ds3")
      , ds4("ds4")
      , ds5("ds5")
    {
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
        add(ds4, end_Devent, ds5);
    }

    State* getStartState() { return &ds1; }
    State* getStopState() { return &ds5; }

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
    Event o_in;
    Event o_out;
    Event end_Devent;
};
} // namespace tsmtest
