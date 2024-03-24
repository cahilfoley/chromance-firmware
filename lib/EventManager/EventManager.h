#ifndef EventManager_h
#define EventManager_h

#include <algorithm>
#include <map>
#include <vector>

class EventManager {
 private:
  std::map<std::string, std::vector<void (*)(int)> > events;

 public:
  EventManager() {}

  EventManager *on(std::string event_name, void (*callback)(int)) {
    // we're using a pointer to reference `events[event_name]` so as
    // to get reference to original object and not the copy object.
    std::vector<void (*)(int)> *listeners = &events[event_name];

    // if this listener is already registered, we wont add it again
    if (std::find(listeners->begin(), listeners->end(), callback) !=
        listeners->end()) {
      return this;
    }

    listeners->push_back(callback);

    return this;
  }

  bool emit(std::string event_name, int arg) {
    std::vector<void (*)(int)> listeners = events[event_name];

    if (listeners.size() == 0) return false;

    // Run all the listeners associated with the event
    for (int idx = 0; idx < listeners.size(); idx += 1) {
      listeners[idx](arg);
    }

    return true;
  }
};

#endif