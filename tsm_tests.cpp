#include <future>
#include <gtest/gtest.h>
#include <set>
#include <utility>

#include "Event.h"
#include "EventQueue.h"
#include "State.h"
#include "tsm.h"
#include <glog/logging.h>

using tsm::Event;
using tsm::EventQueue;
using tsm::State;
using tsm::StateMachine;

struct StateMachineTest : public StateMachine
{
    StateMachineTest() = delete;
    StateMachineTest(std::string name,
                     std::shared_ptr<State> startState,
                     std::shared_ptr<State> stopState,
                     EventQueue<Event>& eventQueue)
      : StateMachine(name, startState, stopState, eventQueue)
    {}

    virtual ~StateMachineTest() = default;

    std::shared_ptr<State> const& getCurrentState() const override
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
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

struct TestStateMachine : public ::testing::Test
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

    // The StateMachine
    // Starting State: doorClosed
    // Stop State: doorOpen
    std::shared_ptr<StateMachineTest> sm = std::make_shared<StateMachineTest>(
      "Garage Door SM", doorClosed, doorDummyFinal, eventQueue);

    // TransitionTable
    sm->add(doorClosed, click_event, doorOpening);
    sm->add(doorOpening, topSensor_event, doorOpen);
    sm->add(doorOpen, click_event, doorClosing);
    sm->add(doorClosing, bottomSensor_event, doorClosed);
    sm->add(doorOpening, click_event, doorStoppedOpening);
    sm->add(doorStoppedOpening, click_event, doorClosing);
    sm->add(doorClosing, obstruct_event, doorStoppedClosing);
    sm->add(doorClosing, click_event, doorStoppedClosing);
    sm->add(doorStoppedClosing, click_event, doorOpening);
    sm->add(doorClosed, click_event, doorOpening);

    sm->start();
    EXPECT_EQ(sm->getCurrentState(), doorClosed);
    eventQueue.addEvent(click_event);
    EXPECT_EQ(doorOpening, sm->getCurrentState());
    eventQueue.addEvent(topSensor_event);
    EXPECT_EQ(doorOpen, sm->getCurrentState());
    sm->stop();
}

struct CdPlayerController
{
    // Actions
    void play(Event const&) { LOG(ERROR) << __PRETTY_FUNCTION__; }
};

struct CdPlayerHSM : public StateMachineTest
{
    CdPlayerHSM() = delete;
    CdPlayerHSM(std::string name,
                std::shared_ptr<State> startState,
                std::shared_ptr<State> stopState,
                EventQueue<Event>& eventQueue)
      : StateMachineTest(name, startState, stopState, eventQueue)
    {}

    virtual ~CdPlayerHSM() = default;
};

// TODO(sriram): Test no end state
// Model interruptions to workflow
// For e.g. As door is opening or closing, one of the sensors
// detects an obstacle.
struct TestCdPlayer : public testing::Test
{
    TestCdPlayer()
      : testing::Test()
      , Stopped(std::make_shared<State>("Player Stopped"))
      , Song1(std::make_shared<State>("Playing HSM -> Song1"))
      , Song2(std::make_shared<State>("Playing HSM -> Song2"))
      , Song3(std::make_shared<State>("Playing HSM -> Song3"))
      , FinalPlay(std::make_shared<State>("Playing Final State"))
      , Playing(std::make_shared<StateMachineTest>("Playing HSM",
                                                   Song1,
                                                   FinalPlay,
                                                   eventQueue))
      , Paused(std::make_shared<State>("Player Paused"))
      , Empty(std::make_shared<State>("Player Empty"))
      , Open(std::make_shared<State>("Player Open"))
      , Final(std::make_shared<State>("Player Final"))
      , sm(std::make_shared<StateMachineTest>("CD Player HSM",
                                              Empty,
                                              Final,
                                              eventQueue))
    {

        // Transition Table for Playing HSM
        Playing->add(Song1, next_song, Song2);
        Playing->add(Song2, next_song, Song3);
        Playing->add(Song3, prev_song, Song2);
        Playing->add(Song2, prev_song, Song1);

        // TransitionTable for GarageDoor HSM
        sm->add(Stopped, play, Playing);
        sm->add(Stopped, open_close, Open);
        sm->add(Stopped, stop, Stopped);
        //-------------------------------------------------
        sm->add(Open, open_close, Empty);
        //-------------------------------------------------
        sm->add(Empty, open_close, Open);
        sm->add(Empty, cd_detected, Stopped);
        sm->add(Empty, cd_detected, Playing);
        //-------------------------------------------------
        sm->add(Playing, stop, Stopped);
        sm->add(Playing, pause, Paused);
        sm->add(Playing, open_close, Open);
        //-------------------------------------------------
        sm->add(Paused, end_pause, Playing);
        sm->add(Paused, stop, Stopped);
        sm->add(Paused, open_close, Open);
    }

    // Event Queue
    EventQueue<Event> eventQueue;

    // CdPlayer HSM
    // States
    shared_ptr<State> Stopped;

    // Playing HSM (nested)
    // States
    shared_ptr<State> Song1;
    shared_ptr<State> Song2;
    shared_ptr<State> Song3;
    shared_ptr<State> FinalPlay;
    // Events
    Event next_song;
    Event prev_song;

    shared_ptr<StateMachine> Playing;

    shared_ptr<State> Paused;
    shared_ptr<State> Empty;
    shared_ptr<State> Open;
    shared_ptr<State> Final;

    // Events
    Event play;
    Event open_close;
    Event stop;
    Event cd_detected;
    Event pause;
    Event end_pause;

    // The StateMachine
    // Starting State: Empty
    shared_ptr<StateMachineTest> sm;
};

TEST_F(TestCdPlayer, testCdPlayerHSM)
{
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
    ASSERT_EQ(sm->getCurrentState(), Paused);
    ASSERT_EQ(Playing->getCurrentState(), Song2);

    eventQueue.addEvent(end_pause);
    ASSERT_EQ(sm->getCurrentState(), Playing);

    sm->stop();
}

TEST_F(TestStateMachine, testCdPlayerHSMWithActions) {}

int
main(int argc, char* argv[])
{
    google::InitGoogleLogging(argv[0]);
    google::InstallFailureSignalHandler();
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
