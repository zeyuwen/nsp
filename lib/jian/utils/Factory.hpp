#pragma once

#include <map>
#include <functional>
#include <string>

namespace jian {

template<typename T>
class Factory;

template<typename T, typename... Args>
class Factory<T*(Args...)> {
public:
    using callback_t = std::function<T*(Args...)>;
    using callback_map_t = std::map<std::string, callback_t>;

    callback_map_t s_methods;

    static Factory<T*(Args...)> &instance() {
        static Factory<T*(Args...)> factory;
        return factory;
    }

    template<typename U>
    struct register_t {
        register_t(const std::string &s) {
            instance().s_methods[s] = [](Args && ...args)->T*{return new U{args...};};
        }
    };

    static T *create(const std::string &s, Args ...args) {
        if (instance().s_methods.find(s) != instance().s_methods.end()) {
            return instance().s_methods[s](args...);
        } else {
            throw std::string("Factory::create error! Unknown method: ") + s;
        }
    }

};

#define REGISTER_FACTORY(Callback, name, Type) namespace {Factory<Callback>::register_t<Type> reg(name);}

} // namespace jian

