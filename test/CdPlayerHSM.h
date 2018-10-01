#pragma once

#include "tsm.h"

#include <glog/logging.h>
#include <gtest/gtest.h>

using tsm::Event;
using tsm::EventQueue;
using tsm::HSMBehavior;
using tsm::State;
using tsm::StateMachineDef;

namespace tsmtest {

struct CdPlayerController
{
    // Actions
    void playSong(std::string const& songName)
    {
        LOG(ERROR) << "Playing song: " << songName;
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
struct CdPlayerDef : public StateMachineDef<CdPlayerDef<ControllerType>>
{
    using StateMachineDef<CdPlayerDef>::add;

    // Playing HSM
    struct PlayingHSMDef : public StateMachineDef<PlayingHSMDef>
    {
        using StateMachineDef<PlayingHSMDef>::add;

        PlayingHSMDef(State* parent = nullptr)
          : StateMachineDef<PlayingHSMDef>("Playing HSM", parent)
          , Song1(std::make_shared<State>("Playing HSM -> Song1"))
          , Song2(std::make_shared<State>("Playing HSM -> Song2"))
          , Song3(std::make_shared<State>("Playing HSM -> Song3"))
        {

            // clang-format off
            add(Song1, next_song, Song2, &PlayingHSMDef::PlaySong, &PlayingHSMDef::PlaySongGuard);
            // clang-format on
            add(Song2, next_song, Song3);
            add(Song3, prev_song, Song2);
            add(Song2, prev_song, Song1);
        }

        shared_ptr<State> getStartState() const { return Song1; }

        // States
        shared_ptr<State> Song1;
        shared_ptr<State> Song2;
        shared_ptr<State> Song3;

        // Events
        Event next_song;
        Event prev_song;

        // Actions
        void PlaySong() { LOG(INFO) << "Play Song"; }
        bool PlaySongGuard()
        {
            LOG(INFO) << "Play Song Guard";
            return true;
        }

        // If the event is a pause, stay on the song that is playing. For all
        // other events, go back to the start state
        void onEntry(Event const& e) override
        {
            auto parent = static_cast<CdPlayerDef*>(this->parent_);
            if (parent->end_pause != e) {
                StateMachineDef<PlayingHSMDef>::onEntry(e);
            }
        }

        void onExit(Event const& e) override
        {
            auto parent = static_cast<CdPlayerDef*>(this->parent_);
            if (parent->pause != e) {
                StateMachineDef<PlayingHSMDef>::onExit(e);
            }
        }
    };

    CdPlayerDef(State* parent = nullptr)
      : StateMachineDef<CdPlayerDef>("CD Player HSM", parent)
      , Stopped(std::make_shared<State>("Player Stopped"))
      , Playing(std::make_shared<HSMBehavior<PlayingHSMDef>>(this))
      , Paused(std::make_shared<State>("Player Paused"))
      , Empty(std::make_shared<State>("Player Empty"))
      , Open(std::make_shared<State>("Player Open"))
    {
        // TransitionTable for GarageDoor HSM
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
        LOG(INFO) << "****** Adding pause: " << pause.id
                  << "Playing id: " << Playing->id;
        add(Playing, pause, Paused);
        add(Playing, open_close, Open);
        //-------------------------------------------------
        add(Paused, end_pause, Playing);
        add(Paused, stop_event, Stopped);
        add(Paused, open_close, Open);
    }

    virtual ~CdPlayerDef() = default;

    shared_ptr<State> getStartState() const { return Empty; }

    // CdPlayer HSM
    // States
    shared_ptr<State> Stopped;

    shared_ptr<PlayingHSMDef> Playing;

    shared_ptr<State> Paused;
    shared_ptr<State> Empty;
    shared_ptr<State> Open;

    // Events
    Event play;
    Event open_close;
    Event stop_event;
    Event cd_detected;
    Event pause;
    Event end_pause;

    ControllerType controller_;
};

struct ErrorHSM : public StateMachineDef<ErrorHSM>
{
    ErrorHSM(State* parent = nullptr)
      : StateMachineDef<ErrorHSM>("Error HSM", parent)
      , AllOk(std::make_shared<State>("All Ok"))
      , ErrorMode(std::make_shared<State>("Error Mode"))
    {
        add(AllOk, error, ErrorMode);
        // Potentially transition to a recovery HSM
        add(ErrorMode, recover, AllOk, &ErrorHSM::recovery);
    }

    // States
    shared_ptr<State> AllOk;
    shared_ptr<State> ErrorMode;

    // Events
    Event error;
    Event recover;

    // Actions
    void recovery() { LOG(INFO) << "Recovering from Error:"; }

    shared_ptr<State> getStartState() const { return AllOk; }
};

} // namespace tsmtest
