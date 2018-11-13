#pragma once

#include <cstddef>
#include <cassert>
#include <utility>

class SlotBase;

class SignalBase {
    friend SlotBase;
protected:
    SignalBase() = default;
    SignalBase(SignalBase&& from) noexcept;
    SignalBase(const SignalBase&) = delete;

    SlotBase* first_slot = nullptr;

    template<class Closure>
    void foreach_slot(Closure&& closure);
};


class SlotBase {
    friend SignalBase;

    SignalBase* signal = nullptr;
    SlotBase* next = nullptr;
    SlotBase* prev = nullptr;

public:
    explicit SlotBase(std::nullptr_t){}

    explicit SlotBase(SignalBase& signal){
        connect(signal);
    }

    SlotBase(const SlotBase&) = delete;
    SlotBase(SlotBase&& from) noexcept
    {
        SignalBase* signal = from.signal;
        from.disconnect();
        connect(*signal);
    }

    ~SlotBase(){
        disconnect();
    }

protected:
    void connect(SignalBase& new_signal){
        disconnect();

        this->signal = &new_signal;

        SlotBase* initial_first_child = signal->first_slot;
        signal->first_slot = this;

        next = initial_first_child;
        if (initial_first_child) initial_first_child->prev = this;

    }
    void disconnect(){
        // self unregister
        if (signal == nullptr) return;

        if (prev == nullptr && next == nullptr) {
            assert(signal->first_slot == this);
            signal->first_slot = nullptr;
        }
        else if (prev == nullptr)
        {
            assert(signal->first_slot == this);
            signal->first_slot = next;
            next->prev = nullptr;
        }
        else if (next == nullptr)
        {
            prev->next = nullptr;
        }
        else
        {
            SlotBase* initial_prev = prev;
            SlotBase* initial_next = next;

            initial_prev->next = initial_next;
            initial_next->prev = initial_prev;
        }

        signal = nullptr;
    }
};


template<class Closure>
void SignalBase::foreach_slot(Closure&& closure) {
    SlotBase* slot = first_slot;
    while(slot){
        closure(*slot);
        slot = slot->next;
    }
}

SignalBase::SignalBase(SignalBase&& from) noexcept
    : first_slot(from.first_slot)
{
    from.first_slot = nullptr;
    foreach_slot([&](SlotBase& slot){
        slot.signal = this;
    });
}


template<class SlotInterface>
class Signal : public SignalBase {
public:
    template<class Closure, typename = std::enable_if_t< !std::is_member_pointer_v<Closure>  > >
    void emit(Closure&& closure){
        foreach_slot([&](SlotBase& slot){
            closure(
                static_cast<SlotInterface&>(slot)
            );
        });
    }

    // from #https://stackoverflow.com/a/9779391
    template <typename ...Args>
    void emit( void (SlotInterface::*mf)(Args...), Args&&... args){
        emit([&](SlotInterface& slot){
            (slot.*mf)(std::forward<Args>(args)...);
        });
    }
};


