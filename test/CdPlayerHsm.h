#pragma once

#include "tsm.h"

namespace tsmtest {

using tsm::Event;
using tsm::EventQueue;
using tsm::Hsm;
using tsm::IHsm;
using tsm::State;

struct CdPlayerController
{
    // Actions
    void playSong(std::string const& songName)
    {
        LOG(INFO) << "Playing song: " << songName;
    }
};

// Implements the state machine specified in
// https://www.boost.org/doc/libs/1_64_0/libs/msm/doc/HTML/ch03s02.html#d0e344
// Look at the image under "Defining a submachine."
// Use this example to specify your own state machines with actions and guards.

// The ControllerType is not strictly required. It doesn't have to be templated
// as well. I think about it as the "context" associated with the state machine.
// This way the CdPlayerController can encapsulate stuff like mp3 encodings,
// hardware details etc. with the CdPlayerHsm delegating all actions to the
// controller.
template<typename ControllerType>
struct CdPlayerHsm : public Hsm<CdPlayerHsm<ControllerType>>
{
    using Hsm<CdPlayerHsm>::add;

    // Playing Hsm
    struct PlayingHsm : public Hsm<PlayingHsm>
    {
        using Hsm<PlayingHsm>::add;

        PlayingHsm()
        {
            IHsm::setStartState(&Song1);

            // clang-format off
            add(Song1, next_song, Song2, &PlayingHsm::PlaySong, &PlayingHsm::PlaySongGuard);
            // clang-format on
            add(Song2, next_song, Song3);
            add(Song3, prev_song, Song2);
            add(Song2, prev_song, Song1);
        }


        // States
        State Song1, Song2, Song3;

        // Events
        Event next_song, prev_song;

        // Actions
        void PlaySong()
        {
            LOG(INFO) << "Play Song";
            controller_.playSong("Dummy");
        }

        // Guards
        bool PlaySongGuard()
        {
            LOG(INFO) << "Play Song Guard";
            return true;
        }

        // If the event is a pause, stay on the song that is playing. For all
        // other events, go back to the start state
        void onEntry(Event const& e) override
        {
            auto parent = static_cast<CdPlayerHsm*>(this->getParent());
            if (parent->end_pause != e) {
                Hsm<PlayingHsm>::onEntry(e);
            }
        }

        void onExit(Event const& e) override
        {
            auto parent = static_cast<CdPlayerHsm*>(this->getParent());
            if (parent->pause != e) {
                Hsm<PlayingHsm>::onExit(e);
            }
        }

        CdPlayerController controller_;
    };

    CdPlayerHsm()
    {
        IHsm::setStartState(&Empty);

        Playing.setParent(this);

        // State Transition Table
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

    // CdPlayer Hsm
    // States
    State Stopped, Paused, Empty, Open;

    // Playing is a Hsm and a State
    PlayingHsm Playing;

    // Events
    Event play, open_close, stop_event, cd_detected, pause, end_pause;

    ControllerType controller_;
};

struct ErrorHsm : public Hsm<ErrorHsm>
{
    ErrorHsm()
    {
        IHsm::setStartState(&AllOk);

        add(AllOk, error, ErrorMode);
        // Potentially transition to a recovery Hsm
        add(ErrorMode, recover, AllOk, &ErrorHsm::recovery);
    }

    // States
    State AllOk, ErrorMode;

    // Events
    Event error, recover;

    // Actions
    void recovery() { LOG(INFO) << "Recovering from Error:"; }
};

} // namespace tsmtest
