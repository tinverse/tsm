#include <future>
#include <gtest/gtest.h>
#include <set>
#include <utility>

#include "Event.h"
#include "EventQueue.h"
#include "State.h"
#include "tsm.h"
#include <glog/logging.h>

using namespace tsm;

class StateMachineTest : public StateMachine
{
  public:
    StateMachineTest() = delete;
    StateMachineTest(std::string name,
                     std::shared_ptr<State> startState,
                     std::shared_ptr<State> stopState,
                     EventQueue<Event>& eventQueue,
                     StateTransitionTable table)
      : StateMachine(name, startState, stopState, eventQueue, table)
    {}

    std::shared_ptr<State> const& getCurrentState() const override
    {
        std::this_thread::yield();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        return currentState_;
    }
};

class TestState : public testing::Test
{
  public:
    TestState()
      : state_("Dummy")
    {}
    ~TestState() override = default;
    void SetUp() override {}
    void TearDown() override {}

  protected:
    State state_;
};

TEST_F(TestState, Construct)
{
    DLOG(INFO) << "Test" << std::endl;
}

TEST_F(TestState, Call)
{
    state_.execute();
}

class TestEventQueue : public testing::Test
{
  public:
    ~TestEventQueue() override = default;

  protected:
    EventQueue<Event> eq_;
    Event e1, e2, e3;
};

TEST_F(TestEventQueue, testSingleEvent)
{
    auto f1 = std::async(&EventQueue<Event>::nextEvent, &eq_);

    std::thread t1(&EventQueue<Event>::addEvent, &eq_, e1);

    // Use the same threads to retrieve events
    Event actualEvent1 = f1.get();
    t1.join();
    EXPECT_EQ(actualEvent1.id, e1.id);
}

TEST_F(TestEventQueue, testAddFrom100Threads)
{
    EventQueue<Event> eq_;
    std::vector<Event> v;
    const int NEVENTS = 100;
    v.reserve(NEVENTS);

    for (int i = 0; i < NEVENTS; i++) {
        v.emplace_back();
    }

    std::vector<std::thread> vtProduce;
    std::vector<std::future<const Event>> vtConsume;

    for (auto event : v) {
        vtConsume.push_back(std::async(&EventQueue<Event>::nextEvent, &eq_));
        vtProduce.emplace_back(&EventQueue<Event>::addEvent, &eq_, event);
    }

    for (auto&& future : vtConsume) {
        const Event e = future.get();
        ASSERT_TRUE(std::find(v.begin(), v.end(), e) != v.end());
    }

    for (auto&& t : vtProduce) {
        t.join();
    }
}

// TODO(sriram): Test no end state
// Model interruptions to workflow
// For e.g. As door is opening or closing, one of the sensors
// detects an obstacle.
class TestStateMachine : public testing::Test
{};

TEST_F(TestStateMachine, testGarageDoor)
{
    // States
    auto doorOpen = std::make_shared<State>("Door Open");
    auto doorOpening = std::make_shared<State>("Door Opening");
    auto doorClosed = std::make_shared<State>("Door Closed");
    auto doorClosing = std::make_shared<State>("Door Closing");
    auto doorStoppedClosing = std::make_shared<State>("Door Stopped Closing");
    auto doorStoppedOpening = std::make_shared<State>("Door Stopped Opening");
    auto doorDummyFinal = std::make_shared<State>("Door Final");

    // Events
    Event click_event;
    Event bottomSensor_event;
    Event topSensor_event;
    Event obstruct_event;

    // Event Queue
    EventQueue<Event> eventQueue;

    // TransitionTable
    StateTransitionTable garageDoorTransitions;

    // Add Transitions to the table
    garageDoorTransitions.add(doorClosed, click_event, doorOpening);
    garageDoorTransitions.add(doorOpening, topSensor_event, doorOpen);
    garageDoorTransitions.add(doorOpen, click_event, doorClosing);
    garageDoorTransitions.add(doorClosing, bottomSensor_event, doorClosed);
    garageDoorTransitions.add(doorOpening, click_event, doorStoppedOpening);
    garageDoorTransitions.add(doorStoppedOpening, click_event, doorClosing);
    garageDoorTransitions.add(doorClosing, obstruct_event, doorStoppedClosing);
    garageDoorTransitions.add(doorClosing, click_event, doorStoppedClosing);
    garageDoorTransitions.add(doorStoppedClosing, click_event, doorOpening);
    garageDoorTransitions.add(doorClosed, click_event, doorOpening);

    LOG(INFO) << "*******Num Transitions:" << garageDoorTransitions.size();

    // The StateMachine
    // Starting State: doorClosed
    // Stop State: doorOpen
    std::shared_ptr<StateMachineTest> sm =
      std::make_shared<StateMachineTest>("Garage Door SM",
                                         doorClosed,
                                         doorDummyFinal,
                                         eventQueue,
                                         garageDoorTransitions);
    sm->start();
    EXPECT_EQ(sm->getCurrentState(), doorClosed);
    eventQueue.addEvent(click_event);
    EXPECT_EQ(doorOpening, sm->getCurrentState());
    eventQueue.addEvent(topSensor_event);
    EXPECT_EQ(doorOpen, sm->getCurrentState());
    sm->stop();
}

TEST_F(TestStateMachine, testCdPlayer)
{
    // States
    auto Stopped = std::make_shared<State>("Player Stopped");
    auto Playing = std::make_shared<State>("Player Playing");
    auto Paused = std::make_shared<State>("Player Paused");
    auto Empty = std::make_shared<State>("Player Empty");
    auto Open = std::make_shared<State>("Player Open");
    auto Final = std::make_shared<State>("Player Final");

    // Events
    Event play;
    Event open_close;
    Event stop;
    Event cd_detected;
    Event pause;
    Event end_pause;

    // Event Queue
    EventQueue<Event> eventQueue;

    // TransitionTable
    StateTransitionTable cdPlayerTransitions;

    // Add Transitions to the table
    cdPlayerTransitions.add(Stopped, play, Playing);
    cdPlayerTransitions.add(Stopped, open_close, Open);
    cdPlayerTransitions.add(Stopped, stop, Stopped);
    //-------------------------------------------------
    cdPlayerTransitions.add(Open, open_close, Empty);
    //-------------------------------------------------
    cdPlayerTransitions.add(Empty, open_close, Open);
    cdPlayerTransitions.add(Empty, cd_detected, Stopped);
    cdPlayerTransitions.add(Empty, cd_detected, Playing);
    //-------------------------------------------------
    cdPlayerTransitions.add(Playing, stop, Stopped);
    cdPlayerTransitions.add(Playing, pause, Paused);
    cdPlayerTransitions.add(Playing, open_close, Open);
    //-------------------------------------------------
    cdPlayerTransitions.add(Paused, end_pause, Playing);
    cdPlayerTransitions.add(Paused, stop, Stopped);
    cdPlayerTransitions.add(Paused, open_close, Open);

    // The StateMachine
    // Starting State: Empty
    std::shared_ptr<StateMachineTest> sm = std::make_shared<StateMachineTest>(
      "CD Player HSM", Empty, Final, eventQueue, cdPlayerTransitions);
    sm->start();

    eventQueue.addEvent(open_close);
    ASSERT_EQ(sm->getCurrentState(), Open);

    eventQueue.addEvent(open_close);
    ASSERT_EQ(sm->getCurrentState(), Empty);

    eventQueue.addEvent(cd_detected);
    ASSERT_EQ(sm->getCurrentState(), Stopped);

    eventQueue.addEvent(play);
    ASSERT_EQ(sm->getCurrentState(), Playing);

    eventQueue.addEvent(pause);
    ASSERT_EQ(sm->getCurrentState(), Paused);

    eventQueue.addEvent(stop);
    ASSERT_EQ(sm->getCurrentState(), Stopped);

    sm->stop();
}

TEST_F(TestStateMachine, testCdPlayerHSM)
{
    // Playing HSM
    // States
    auto Song1 = std::make_shared<State>("Playing HSM -> Song1");
    auto Song2 = std::make_shared<State>("Playing HSM -> Song2");
    auto Song3 = std::make_shared<State>("Playing HSM -> Song3");
    auto FinalPlay = std::make_shared<State>("Playing Final State");
    // Events
    Event next_song;
    Event prev_song;

    // Event Queue
    EventQueue<Event> eventQueue;

    // Transition Table
    StateTransitionTable playTransitions;
    playTransitions.add(Song1, next_song, Song2);
    playTransitions.add(Song2, next_song, Song3);
    playTransitions.add(Song3, prev_song, Song2);
    playTransitions.add(Song2, prev_song, Song1);

    // States
    auto Stopped = std::make_shared<State>("Player Stopped");
    auto Playing = std::make_shared<StateMachineTest>(
      "Playing HSM", Song1, FinalPlay, eventQueue, playTransitions);
    auto Paused = std::make_shared<State>("Player Paused");
    auto Empty = std::make_shared<State>("Player Empty");
    auto Open = std::make_shared<State>("Player Open");
    auto Final = std::make_shared<State>("Player Final");

    // Events
    Event play;
    Event open_close;
    Event stop;
    Event cd_detected;
    Event pause;
    Event end_pause;

    // TransitionTable
    StateTransitionTable cdPlayerTransitions;

    // Add Transitions to the table
    cdPlayerTransitions.add(Stopped, play, Playing);
    cdPlayerTransitions.add(Stopped, open_close, Open);
    cdPlayerTransitions.add(Stopped, stop, Stopped);
    //-------------------------------------------------
    cdPlayerTransitions.add(Open, open_close, Empty);
    //-------------------------------------------------
    cdPlayerTransitions.add(Empty, open_close, Open);
    cdPlayerTransitions.add(Empty, cd_detected, Stopped);
    cdPlayerTransitions.add(Empty, cd_detected, Playing);
    //-------------------------------------------------
    cdPlayerTransitions.add(Playing, stop, Stopped);
    cdPlayerTransitions.add(Playing, pause, Paused);
    cdPlayerTransitions.add(Playing, open_close, Open);
    //-------------------------------------------------
    cdPlayerTransitions.add(Paused, end_pause, Playing);
    cdPlayerTransitions.add(Paused, stop, Stopped);
    cdPlayerTransitions.add(Paused, open_close, Open);

    // The StateMachine
    // Starting State: Empty
    auto sm = std::make_shared<StateMachineTest>(
      "CD Player HSM", Empty, Final, eventQueue, cdPlayerTransitions);

    ASSERT_EQ(Playing->getParent(), sm.get());

    sm->start();

    eventQueue.addEvent(cd_detected);
    ASSERT_EQ(sm->getCurrentState(), Stopped);

    eventQueue.addEvent(play);
    ASSERT_EQ(sm->getCurrentState(), Playing);
    ASSERT_EQ(Playing->getCurrentState(), Song1);

    eventQueue.addEvent(next_song);
    ASSERT_EQ(sm->getCurrentState(), Playing);
    ASSERT_EQ(Playing->getCurrentState(), Song2);

    eventQueue.addEvent(pause);
    sm->stop();
}

// Boost HSM record player example

int
main(int argc, char* argv[])
{
    google::InitGoogleLogging(argv[0]);
    google::InstallFailureSignalHandler();
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
