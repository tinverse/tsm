#include "tsm_log.h"

#include "TestMachines.h"

#include <catch2/catch.hpp>

#include <memory>

using tsm::Event;

using tsmtest::AHsm;

TEST_CASE("State machine drill")
{

    SingleThreadedHsm<AHsm> sm;
    sm.startSM();
    REQUIRE(&sm.s1 == sm.getCurrentState());
    sm.sendEvent(sm.e1);
    sm.step();
    REQUIRE(&sm.s2 == sm.getCurrentState());
    sm.sendEvent(sm.e2_in);
    sm.step();
    REQUIRE(&sm.bHsm == sm.getCurrentState());
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
    sm.stopSM();
}

TEST_CASE("State machine drill - Exit before processing all events")
{

    SingleThreadedHsm<AHsm> sm;
    sm.startSM();
    REQUIRE(&sm.s1 == sm.getCurrentState());
    sm.sendEvent(sm.e1);
    sm.step();
    REQUIRE(&sm.s2 == sm.getCurrentState());
    sm.sendEvent(sm.e2_in);
    sm.step();
    REQUIRE(&sm.bHsm == sm.getCurrentState());
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
    sm.stopSM();
}

TEST_CASE("State machine drill - Asynchronous")
{

    using namespace std::chrono_literals;
    AsynchronousHsm<AHsm> sm;
    sm.startSM();
    REQUIRE(&sm.s1 == sm.getCurrentState());
    sm.sendEvent(sm.e1);
    sm.sendEvent(sm.e2_in);
    sm.sendEvent(sm.e2_out);
    sm.sendEvent(sm.e2_out);
    Event randomEvent;
    sm.sendEvent(randomEvent);
    sm.sendEvent(sm.end_event);
    // Not much of a test, but the asynchronous SM should
    // process the events and terminate. We need *some* test
    // code that exercises AsynchronousHsm even though there
    // are several tests that use a BlockingObserver, essentially
    // turning an AsynchronousHsm into a Synchronous one for
    // testing purposes.
    while (nullptr != sm.getCurrentState()) {
        std::this_thread::sleep_for(1ms);
    }
    REQUIRE(nullptr == sm.getCurrentState());
    sm.stopSM();
}

// Above test written the "BDD way". Writing for practice.
SCENARIO("Test that a state machine transitions through its states to sub-Hsms "
         "and reaches its final state")
{
    GIVEN("A simple state machine")
    {
        SingleThreadedHsm<AHsm> sm;
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
                    THEN("the state machine transitions to the sub Hsm bHsm")
                    {
                        REQUIRE(&sm.bHsm == sm.getCurrentState());
                    }
                    WHEN(" Event e2_out is sent")
                    {
                        sm.sendEvent(sm.e2_out);
                        sm.step();
                        THEN("the state machine transitions out of sub Hsm "
                             "bHsm into s3")
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
