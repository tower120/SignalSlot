#include <iostream>
#include "SignalSlot.h"

namespace Test{

    struct Changeable : SlotBase {
        using SlotBase::SlotBase;
        virtual void added() = 0;
        virtual void deleted(int i) = 0;
        virtual void deleted(int i, int& i2) = 0;
    };


    Signal<Changeable> signal;


    struct A {
        struct ChangeableSlot : Changeable {
            using Changeable::Changeable;
            using SlotBase::disconnect;
            void added() override {
                std::cout << "A : added" << std::endl;
            }
            void deleted(int i) override {
                std::cout << "A : deleted " << std::to_string(i) << std::endl;
            }
            void deleted(int i, int& i2) override {
                std::cout << "A : deleted "
                          << std::to_string(i) << " "
                          << std::to_string(i2)
                          << std::endl;
            }

        } slot {signal};
    };
}

int main() {
    using namespace Test;
    A a;
    Test::signal.emit([](Changeable& _){ _.added();});
    int i = 3;
    Signal<Changeable> signal2 = std::move(Test::signal);
    signal2.emit(&Changeable::added);

    A b = std::move(a);
    signal2.emit(&Changeable::deleted, 2, i);

    b.slot.disconnect();
    signal2.emit(&Changeable::deleted, 22, i);


    return 0;
}