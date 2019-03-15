#pragma once

#include "tsm.h"

using tsm::Event;
using tsm::EventQueue;
using tsm::HsmDefinition;
using tsm::HsmExecutor;
using tsm::IHsmDef;
using tsm::State;

namespace tsmtest {

struct CdPlayerController
{
    // Actions
    void playSong(std::string const& songName)
    {
        DLOG(INFO) << "Playing song: " << songName;
    }
};

// Implements the state machine specified in
// https://www.boost.org/doc/libs/1_64_0/libs/msm/doc/HTML/ch03s02.html#d0e344
// Look at the image under "Defining a submachine."
// Use this example to specify your own state machines with actions and guards.

// The ControllerType is not strictly required. It doesn't have to be templated
// as well. I think about it as the "context" associated with the state machine.
// This way the CdPlayerController can encapsulate stuff like mp3 encodings,
// hardware details etc. with the CdPlayerDef delegating all actions to the
// controller.
template<typename ControllerType>
struct CdPlayerDef : public HsmDefinition<CdPlayerDef<ControllerType>>
{
    using HsmDefinition<CdPlayerDef>::add;

    // Playing Hsm
    struct PlayingHsmDef : public HsmDefinition<PlayingHsmDef>
    {
        using HsmDefinition<PlayingHsmDef>::add;

        struct Song1State : public State
        {
            Song1State()
              : State("Playing Hsm -> Song1")
            {}
            void execute(Event const&) override {}
        };
        PlayingHsmDef(IHsmDef* parent = nullptr)
          : HsmDefinition<PlayingHsmDef>("Playing Hsm", parent)
          , Song1()
          , Song2("Playing Hsm -> Song2")
          , Song3("Playing Hsm -> Song3")
        {

            // clang-format off
            add(Song1, next_song, Song2, &PlayingHsmDef::PlaySong, &PlayingHsmDef::PlaySongGuard);
            // clang-format on
            add(Song2, next_song, Song3);
            add(Song3, prev_song, Song2);
            add(Song2, prev_song, Song1);
        }

        virtual ~PlayingHsmDef() = default;

        State* getStartState() override { return &Song1; }
        State* getStopState() override { return nullptr; }

        // States
        Song1State Song1;
        State Song2;
        State Song3;

        // Events
        Event next_song;
        Event prev_song;

        // Actions
        void PlaySong()
        {
            DLOG(INFO) << "Play Song";
            controller_.playSong(this->getCurrentState()->name);
        }

        // Guards
        bool PlaySongGuard()
        {
            DLOG(INFO) << "Play Song Guard";
            return true;
        }

        CdPlayerController controller_;
    };

    struct PlayingHsmExecutor : public HsmExecutor<PlayingHsmDef>
    {
        PlayingHsmExecutor(IHsmDef* parent = nullptr)
          : HsmExecutor<PlayingHsmDef>(parent)
        {}

        // If the event is a pause, stay on the song that is playing. For all
        // other events, go back to the start state
        void onEntry(Event const& e) override
        {
            auto parent = static_cast<CdPlayerDef*>(this->parent_);
            if (parent->end_pause != e) {
                HsmExecutor<PlayingHsmDef>::onEntry(e);
            }
        }

        void onExit(Event const& e) override
        {
            auto parent = static_cast<CdPlayerDef*>(this->parent_);
            if (parent->pause != e) {
                HsmExecutor<PlayingHsmDef>::onExit(e);
            }
        }
    };

    CdPlayerDef(IHsmDef* parent = nullptr)
      : HsmDefinition<CdPlayerDef>("CD Player Hsm", parent)
      , Stopped("Player Stopped")
      , Playing(this)
      , Paused("Player Paused")
      , Empty("Player Empty")
      , Open("Player Open")
    {
        // TransitionTable for GarageDoor Hsm
        add(Stopped, play, Playing);
        add(Stopped, open_close, Open);
        add(Stopped, stop_event, Stopped);
        //-------------------------------------------------
        add(Open, open_close, Empty);
        //-------------------------------------------------
        add(Empty, open_close, Open);
        add(Empty, cd_detected, Stopped);
        add(Empty, cd_detected, Playing);
        //-------------------------------------------------
        add(Playing, stop_event, Stopped);
        add(Playing, pause, Paused);
        add(Playing, open_close, Open);
        //-------------------------------------------------
        add(Paused, end_pause, Playing);
        add(Paused, stop_event, Stopped);
        add(Paused, open_close, Open);
    }

    virtual ~CdPlayerDef() = default;

    State* getStartState() { return &Empty; }
    State* getStopState() { return nullptr; }

    // CdPlayer Hsm
    // States
    State Stopped;

    PlayingHsmExecutor Playing;

    State Paused;
    State Empty;
    State Open;

    // Events
    Event play;
    Event open_close;
    Event stop_event;
    Event cd_detected;
    Event pause;
    Event end_pause;

    ControllerType controller_;
};

struct ErrorHsm : public HsmDefinition<ErrorHsm>
{
    ErrorHsm(IHsmDef* parent = nullptr)
      : HsmDefinition<ErrorHsm>("Error Hsm", parent)
      , AllOk("All Ok")
      , ErrorMode("Error Mode")
    {
        add(AllOk, error, ErrorMode);
        // Potentially transition to a recovery Hsm
        add(ErrorMode, recover, AllOk, &ErrorHsm::recovery);
    }

    virtual ~ErrorHsm() = default;
    // States
    State AllOk;
    State ErrorMode;

    // Events
    Event error;
    Event recover;

    // Actions
    void recovery() { DLOG(INFO) << "Recovering from Error:"; }

    State* getStartState() { return &AllOk; }
    State* getStopState() { return nullptr; }
};

} // namespace tsmtest
