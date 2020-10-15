# UnitMaker
#### Near zero-cost, header-only abstractions for handling units in idiomatic C\++20
-----
UnitMaker provides the ability to define units of specified base types, and then further refine by multiplying by a ratio (eg. kg to lb), offseting by a fraction (eg. K to &deg;C), inverting (eg. s to Hz), and combining units together (eg. N and m to J). Furthermore it allows for implicitly converting between untis of equivalent base type (eg. N and lbf), allowing for easy to write and easier to read functions and classes. It is a header-only library, and thus can be included inside a namespace for easy encapsulation. ```units.h``` provides the basic functionality, and ```si_units.h``` provides a non-comprehensive implementation of si units and more.

## Examples
-----
### Implicit Conversion
```c++
#include <si_units.h>
#include <iostream>

void display_force(Newton force) {
    std::cout << force.value << " Newtons\n";
}

int main() {
    display_force(Newton{5});               // "5 Newtons"
    // Pounds and Newtons both units of force, able to implicitly convert
    display_force(Pound{5});                // "22.2411 Newtons"
    // Creates unit type via division overload, still a force unit
    display_force(Joule{5} / Foot{0.5});    // "32.8084 Newtons"
    // compile error, EquivalentBaseType not satisfied
    // display_force(Meter{5});
    return 0;
}
```