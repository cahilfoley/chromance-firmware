#ifndef EventEmitter_h
#define EventEmitter_h

#include <functional>
#include <map>
#include <memory>
#include <typeindex>
#include <typeinfo>
#include <vector>

class EventEmitter {
 private:
  class BaseListener {
   public:
    explicit BaseListener() = default;
    BaseListener(const BaseListener&) = delete;
    BaseListener& operator=(const BaseListener&) = delete;
    virtual ~BaseListener() {}
  };

  template <typename Type>
  class Listener : public BaseListener {
   public:
    explicit Listener(const std::function<void(Type&, bool)>& callback) { this->callback = callback; }
    Listener(const Listener&) = delete;
    Listener& operator=(const Listener&) = delete;
    virtual ~Listener() {}
    void emit(Type& type, bool fromHA = false) {
      try {
        callback(type, fromHA);
      } catch (...) {
      }
    }

   private:
    std::function<void(Type&, bool)> callback;
  };

 public:
  template <typename Type>
  void emit(Type& type, bool fromHA = false) {
    auto it = listeners.find(typeid(type));
    if (it != listeners.end()) {
      for (auto& listener : it->second) {
        static_cast<Listener<Type>*>(listener.get())->emit(type, fromHA);
      }
    }
  }

  template <typename Type>
  EventEmitter& on(const std::function<void(Type&, bool)>& callback) {
    if (listeners.find(typeid(Type)) == listeners.end()) {
      listeners[typeid(Type)] = std::vector<std::unique_ptr<BaseListener>>();
    }
    listeners[typeid(Type)].push_back(std::unique_ptr<BaseListener>(new Listener<Type>(callback)));
    return *this;
  }

 private:
  std::map<std::type_index, std::vector<std::unique_ptr<BaseListener>>> listeners;
};

#endif