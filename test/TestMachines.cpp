#include "TestMachines.h"
#include "tsm.h"
#include "tsm_log.h"

#include <catch2/catch.hpp>

#include <memory>

using tsm::Event;
using tsm::State;
using tsm::HsmDefinition;

using tsmtest::AHsmDef;
using tsmtest::BHsmDef;

TEST_CASE("State machine drill")
{

    SingleThreadedHsm<AHsmDef> sm;
    sm.startSM();
    REQUIRE(&sm.s1 == sm.getCurrentState());
    sm.sendEvent(sm.e1);
    sm.step();
    REQUIRE(&sm.s2 == sm.getCurrentState());
    sm.sendEvent(sm.e2_in);
    sm.step();
    REQUIRE(&sm.bHsmDef == sm.getCurrentState());
    sm.sendEvent(sm.e2_out);
    sm.step();
    REQUIRE(&sm.s3 == sm.getCurrentState());
    sm.sendEvent(sm.e2_out);
    sm.step();
    REQUIRE(&sm.s3 == sm.getCurrentState());
    Event randomEvent;
    sm.sendEvent(randomEvent);
    sm.step();
    REQUIRE(&sm.s3 == sm.getCurrentState());
    sm.sendEvent(sm.end_event);
    sm.step();
    REQUIRE(nullptr == sm.getCurrentState());
}

// Above test written the "BDD way". Writing for practice.
SCENARIO("Test that a state machine transitions through its states to sub-Hsms "
         "and reaches its final state")
{
    GIVEN("A simple state machine")
    {
        SingleThreadedHsm<AHsmDef> sm;
        WHEN("started")
        {
            sm.startSM();
            THEN("start state is s1")
            {
                REQUIRE(&sm.s1 == sm.getCurrentState());
            }

            WHEN(" Event e1 is sent")
            {
                sm.sendEvent(sm.e1);
                sm.step();
                THEN("the current state is s2")
                {
                    REQUIRE(&sm.s2 == sm.getCurrentState());
                }
                WHEN(" Event e2_in is sent")
                {
                    sm.sendEvent(sm.e2_in);
                    sm.step();
                    THEN("the state machine transitions to the sub Hsm bHsmDef")
                    {
                        REQUIRE(&sm.bHsmDef == sm.getCurrentState());
                    }
                    WHEN(" Event e2_out is sent")
                    {
                        sm.sendEvent(sm.e2_out);
                        sm.step();
                        THEN("the state machine transitions out of sub Hsm "
                             "bHsmDef into s3")
                        {
                            REQUIRE(&sm.s3 == sm.getCurrentState());
                        }
                        WHEN("An unknown event is sent")
                        {
                            // Send unknown event
                            Event randomEvent;
                            sm.sendEvent(randomEvent);
                            sm.step();
                            THEN("it is ignored and the machines maintains its "
                                 "current state")
                            {
                                REQUIRE(&sm.s3 == sm.getCurrentState());
                            }
                            WHEN("An end event is sent")
                            {
                                sm.sendEvent(sm.end_event);
                                sm.step();
                                THEN("the current state goes to null")
                                {
                                    REQUIRE(nullptr == sm.getCurrentState());
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
