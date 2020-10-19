# UnitMaker
#### Near zero-cost, header-only abstractions for handling units in idiomatic C\++20
-----
UnitMaker provides the ability to define units of specified base types, and then further refine by multiplying by a ratio (eg. kg to lb), offseting by a fraction (eg. K to &deg;C), inverting (eg. s to Hz), and combining units together (eg. N and m to J). Furthermore it allows for implicitly converting between untis of equivalent base type (eg. N and lbf), allowing for easy to write and easier to read functions and classes. It is a header-only library, and thus can be included inside a namespace for easy encapsulation. ```units.h``` provides the basic functionality, and ```si_units.h``` provides a non-comprehensive implementation of si units and more.

## Examples
### Creating New Units
```c++
#include <units.h>
#include <iostream>

// Creating a base type, every length unit defined in relation to Pixel
using Pixel = Unit<BaseTypes::LENGTH>;
using Second = Unit<BaseTypes::TIME>;

// Creating a unit based off of a ratio, in this case one Inch is 96/1 Pixel(s)
using Inch = UnitRatio<Pixel, std::ratio<96, 1>>;

// One Foot is 12/1 Inch is 96/1*12/1 = 1152/1 Pixel(s)
using Foot = UnitRatio<Inch, std::ratio<12, 1>>;

// Inverse units can be created and used as expected
using Hertz = UnitInverse<Second>;

// MultiUnits multiply all the units together
using FeetPerSecond = MultiUnit<Foot, Hertz>;

// SpecificUnits allow specifying the base type and ratio
// Can be useful for defining complicated units
using SevenTimesBaseSeven = SpecifiedUnit<std::ratio<7, 1>, std::ratio<7, 1>>;

// Using primes greater than 13 allows for user defined base_types
using Sievert = SpecifiedUnit<std::ratio<17, 1>>;
```

### Implicit Conversion
```c++
#include <si_units.h>
#include <iostream>

void display_force(Newton force) {
    std::cout << force.value << " Newtons\n";
}

int main() {
    // No conversion needed
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

### Operator Overloads

```c++
#include <si_units.h>
#include <iostream>

void display_meter(Meter unit) {
    cout << unit.value << " Meters\n";
}

int main() {
    // No conversion
    display_meter(Meter{5});                    // "5 Meters"

    // Meter + Foot creates Meter, no conversion
    display_meter(Meter{5} + Foot{5});          // "6.524 Meters"

    // Foot + Meter creates Foot, then converted to Meter
    display_meter(Foot{5} + Meter{5});          // "6.524 Meters"

    // Meter - Foot creates Meter, no conversion
    display_meter(Meter{5} - Foot{5});          // "3.476 Meters"

    // Meter/Second * Second creates (Meter/Second)*Second
    // Not technically meter, but converted at 1:1 ratio
    display_meter(mps{5} * Second{5});          // "25 Meters"

    // (Meter*Meter)/Meter, again converted 1:1 to meter
    display_meter(Square<Meter>{25} / Meter{5});// "5 Meters"
    return 0;
}
```