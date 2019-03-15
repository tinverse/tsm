#include "tsm.h"
using namespace tsm;

namespace tsmtest {

struct BHsmDef : public HsmDefinition<BHsmDef>
{
    BHsmDef(IHsmDef* parent)
      : HsmDefinition<BHsmDef>("BHsmDef", parent)
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

struct AHsmDef : public HsmDefinition<AHsmDef>
{
    AHsmDef(IHsmDef* parent = nullptr)
      : HsmDefinition<AHsmDef>("AHsmDef", parent)
      , bHsmDef(this)
      , s1("s1")
      , s2("s2")
      , s3("s3")
      , s4("s4")
    {
        add(s1, e1, s2);
        add(s2, e2_in, bHsmDef);
        add(bHsmDef, e2_out, s3);
        add(s3, e3, s1);
        add(s3, end_event, s4);
    }

    State* getStartState() { return &s1; }
    State* getStopState() { return &s4; }

    // States
    HsmExecutor<BHsmDef> bHsmDef;
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
}
