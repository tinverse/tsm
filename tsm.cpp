#include <glog/logging.h>
#include <iostream>
#include <utility>

#include "EventQueue.h"
#include "Transition.h"
#include "tsm.h"

using tsm::StateEventPair;

bool
operator==(const StateEventPair& s1, const StateEventPair& s2)
{
    return (s1.first.get() == s2.first.get()) && (s1.second.id == s2.second.id);
}
