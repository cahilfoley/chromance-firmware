#ifndef EventEmitter_h
#define EventEmitter_h

#include <functional>
#include <map>
#include <memory>
#include <typeindex>
#include <typeinfo>

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
    explicit Listener(const std::function<void(Type&)>& callback) { this->callback = callback; }
    Listener(const Listener&) = delete;
    Listener& operator=(const Listener&) = delete;
    virtual ~Listener() {}
    void emit(Type& type) {
      try {
        callback(type);
      } catch (...) {
      }
    }

   private:
    std::function<void(Type&)> callback;
  };

 public:
  template <typename Type>
  void emit(Type& type) {
    auto it = listeners.find(typeid(type));
    if (it != listeners.end()) static_cast<Listener<Type>*>(it->second.get())->emit(type);
  }

  template <typename Type>
  EventEmitter& on(const std::function<void(Type&)>& callback) {
    listeners[typeid(Type)] = std::unique_ptr<BaseListener>(new Listener<Type>(callback));
    return *this;
  }

 private:
  std::map<std::type_index, std::unique_ptr<BaseListener>> listeners;
};

#endif