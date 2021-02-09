#pragma once
#include <vector>
#include <exception>
#include <memory>

class DelegateException : public std::exception {
public:
    const char *what() const noexcept override {
        return "Delegate is empty.";
    }
};

template<typename TResult, typename... TArgs>
class AbstractDelegate {
public:
    virtual TResult operator()(TArgs...) const = 0;

    virtual ~AbstractDelegate() = default;
};

template<typename TReturn, typename... TArgs>
class FunctionDelegate : public AbstractDelegate<TReturn, TArgs...> {
    typedef TReturn(*Func)(TArgs...);

    Func _ptr;

public:
    FunctionDelegate() : _ptr(nullptr) {}

    FunctionDelegate(Func ptr) : _ptr(ptr) {}


    FunctionDelegate &operator=(Func ptr) {
        _ptr = ptr;
    }

    TReturn operator()(TArgs... args) const override {
        if (_ptr == nullptr) {
            throw DelegateException();
        }

        return _ptr(args...);
    }
};

template<typename TReturn, typename TThis, typename... TArgs>
class MemberDelegate : public AbstractDelegate<TReturn, TArgs...> {
    typedef TReturn(TThis::*MemberPtr)(TArgs...);

    MemberPtr _ptr;
    TThis *_obj;
public:
    MemberDelegate() : MemberDelegate(nullptr, nullptr) {}

    MemberDelegate(TThis *obj, MemberPtr ptr) : _ptr(ptr), _obj(obj) {}

    TReturn operator()(TArgs... args) const override {
        return (_obj->*_ptr)(args...);
    }
};

template<typename TReturn, typename... TArgs>
class MulticastDelegate : public AbstractDelegate<TReturn, TArgs...> {
    std::vector<std::shared_ptr<AbstractDelegate<TReturn, TArgs...>>> _delegates;

public:
    MulticastDelegate() = default;

    MulticastDelegate(std::shared_ptr<AbstractDelegate<TReturn, TArgs...>> dele) {
        _delegates.push_back(dele);
    }

    MulticastDelegate &operator+=(std::shared_ptr<AbstractDelegate<TReturn, TArgs...>> delegate) {
        _delegates.push_back(delegate);

        return *this;
    }

    MulticastDelegate &operator-=(std::shared_ptr<AbstractDelegate<TReturn, TArgs...>> delegate) {
        _delegates.erase(std::remove(_delegates.begin(), _delegates.end(), delegate), _delegates.end());

        return *this;
    }

    TReturn operator()(TArgs... args) const override {
        if (_delegates.empty()) {
            throw DelegateException();
        }

        const size_t size = _delegates.size();
        for (size_t i = 0; i < size; i++) {
            if (i == size - 1) {
                return (*_delegates[i])(args...);
            }
            (*_delegates[i])(args...);
        }
    }
};


template<typename TReturn, typename... TArgs>
std::shared_ptr<AbstractDelegate<TReturn, TArgs...>> make_delegate(TReturn(*ptr)(TArgs...)) {
    return std::make_shared<FunctionDelegate<TReturn, TArgs...>>(ptr);
}

template<typename TReturn, typename TThis, typename... TArgs>
std::shared_ptr<AbstractDelegate<TReturn, TArgs...>>
make_delegate(TThis &obj, TReturn(TThis::*ptr)(TArgs...)) {
    return std::make_shared<MemberDelegate<TReturn, TThis, TArgs...>>(&obj, ptr);
}