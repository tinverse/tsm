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

    StateMachineTest(std::string name, EventQueue<Event>& eventQueue)
      : StateMachine(name, eventQueue)
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
      "Garage Door SM", doorClosed, nullptr, eventQueue);

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

    sm->startHSM();
    EXPECT_EQ(sm->getCurrentState(), doorClosed);
    eventQueue.addEvent(click_event);
    EXPECT_EQ(doorOpening, sm->getCurrentState());
    eventQueue.addEvent(topSensor_event);
    EXPECT_EQ(doorOpen, sm->getCurrentState());
    sm->stopHSM();
}

struct CdPlayerController
{
    // Actions
    void playSong(Event const&) { LOG(ERROR) << __PRETTY_FUNCTION__; }
};

struct TestCdPlayer;

template<typename ControllerType>
struct CdPlayerHSM : public StateMachineTest
{
    typedef void (CdPlayerHSM::*ActionType)(const Event&);

    // Playing HSM
    struct PlayingHSM : public StateMachineTest
    {
        PlayingHSM() = delete;
        PlayingHSM(std::string name, EventQueue<Event>& eventQueue)
          : StateMachineTest(name, eventQueue)
          , Song1(std::make_shared<State>("Playing HSM -> Song1"))
          , Song2(std::make_shared<State>("Playing HSM -> Song2"))
          , Song3(std::make_shared<State>("Playing HSM -> Song3"))
        {

            // Transition Table for Playing HSM
            add(Song1, next_song, Song2);
            add(Song2, next_song, Song3);
            add(Song3, prev_song, Song2);
            add(Song2, prev_song, Song1);
        }

        // States
        shared_ptr<State> Song1;
        shared_ptr<State> Song2;
        shared_ptr<State> Song3;

        // Events
        Event next_song;
        Event prev_song;

        shared_ptr<State> getStartState() const override { return Song1; }
    };

    CdPlayerHSM() = delete;
    CdPlayerHSM(std::string name, EventQueue<Event>& eventQueue)
      : StateMachineTest(name, eventQueue)
      , Stopped(std::make_shared<State>("Player Stopped"))
      , Playing(std::make_shared<PlayingHSM>("Playing HSM", eventQueue))
      , Paused(std::make_shared<State>("Player Paused"))
      , Empty(std::make_shared<State>("Player Empty"))
      , Open(std::make_shared<State>("Player Open"))
    {
        // TransitionTable for GarageDoor HSM
        add(Stopped, play, Playing);
        add(Stopped, open_close, Open);
        add(Stopped, stop, Stopped);
        //-------------------------------------------------
        add(Open, open_close, Empty);
        //-------------------------------------------------
        add(Empty, open_close, Open);
        add(Empty, cd_detected, Stopped);
        add(Empty, cd_detected, Playing);
        //-------------------------------------------------
        add(Playing, stop, Stopped);
        add(Playing, pause, Paused);
        add(Playing, open_close, Open);
        //-------------------------------------------------
        add(Paused, end_pause, Playing);
        add(Paused, stop, Stopped);
        add(Paused, open_close, Open);
    }

    virtual ~CdPlayerHSM() = default;

    shared_ptr<State> getStartState() const override { return Empty; }

    // Actions
    void playSong(Event const& e) { controller_.playSong(e); }

    // CdPlayer HSM
    // States
    shared_ptr<State> Stopped;

    shared_ptr<PlayingHSM> Playing;

    shared_ptr<State> Paused;
    shared_ptr<State> Empty;
    shared_ptr<State> Open;

    // Events
    Event play;
    Event open_close;
    Event stop;
    Event cd_detected;
    Event pause;
    Event end_pause;

    ControllerType controller_;
};

// TODO(sriram): Test no end state
// Model interruptions to workflow
// For e.g. As door is opening or closing, one of the sensors
// detects an obstacle.
struct TestCdPlayerHSM : public testing::Test
{
    TestCdPlayerHSM()
      : testing::Test()
      , sm(std::make_shared<CdPlayerHSM<CdPlayerController>>("CD Player HSM",
                                                             eventQueue))
    {}
    // The StateMachine
    // Starting State: Empty
    shared_ptr<CdPlayerHSM<CdPlayerController>> sm;

    // Event Queue
    EventQueue<Event> eventQueue;
};

TEST_F(TestCdPlayerHSM, testTransitions)
{
    auto& Playing = sm->Playing;

    ASSERT_EQ(Playing->getParent(), sm.get());

    sm->startHSM();

    eventQueue.addEvent(sm->cd_detected);
    ASSERT_EQ(sm->getCurrentState(), sm->Stopped);

    eventQueue.addEvent(sm->play);
    ASSERT_EQ(sm->getCurrentState(), Playing);
    ASSERT_EQ(Playing->getCurrentState(), Playing->Song1);

    eventQueue.addEvent(Playing->next_song);
    ASSERT_EQ(sm->getCurrentState(), Playing);
    ASSERT_EQ(Playing->getCurrentState(), Playing->Song2);

    eventQueue.addEvent(sm->pause);
    ASSERT_EQ(sm->getCurrentState(), sm->Paused);
    ASSERT_EQ(Playing->getCurrentState(), Playing->Song2);

    eventQueue.addEvent(sm->end_pause);
    ASSERT_EQ(sm->getCurrentState(), Playing);

    sm->stopHSM();
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
