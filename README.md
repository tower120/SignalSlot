# SignalSlot
Space efficient signal-slot system for common use. Movable! Slot/Signal interface can contatin multiple methods.


Signal costs 1 ptr. Slot costs 3 ptrs.
No heap allocations at all. Fast, O(1) connect/disconnect. All parts movable, but not copyable.
Emit call costs +/-1 virtual call for each registered slot.

Slot autodisconnects on destruction.
Not thread-safe.

**Main feature** - usage of Interface class, instead of smthg like `std::function`.
This allow to put, e.g. 20 methods in one slot at no additional overhead! 

You can embed Slot interface implementation into your class, thus make it safe to move:

```cpp
    struct Changeable : SlotBase {
        using SlotBase::SlotBase;
        virtual void added() = 0;
        virtual void deleted(int i) = 0;
        virtual void deleted(int i, int& i2) = 0;
    };

    Signal<Changeable> signal;

    struct A : Changeable {
        int internal_value  = 20;
    
        void added() override {
            std::cout << "A : added "  << internal_value << std::endl;
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
                
        A() : Changeable(signal) {}
    };
    
    
int main() {
    signal.emit(&Changeable::added);
    signal.emit(&Changeable::deleted, 2, i);
}    
```


Alternative ussage (A probably unmovable now [due to "captured" self]):
```cpp
    struct A {
       int internal_value  = 20;
       
        struct ChangeableSlot : Changeable {
            using Changeable::Changeable;            
            A* self;
            
            void added() override {
                std::cout << "A : added" << self->internal_value << std::endl;
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
            
            using SlotBase::disconnect;     // allow disconnect from outside          
        } slot {nullptr};
        
        A(){
           slot.self = this;
           slot.connect(signal);
        }
    };
```
