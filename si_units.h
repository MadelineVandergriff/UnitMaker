//
// Copyright (c) 2020 Jack Vandergriff.
//

#ifndef UNITMAKER_SI_UNITS_H
#define UNITMAKER_SI_UNITS_H

#include "units.h"

using Kilogram = Unit<BaseTypes::MASS>;
using Meter = Unit<BaseTypes::LENGTH>;
using Second = Unit<BaseTypes::TIME>;
using Kelvin = Unit<BaseTypes::TEMPERATURE>;
using Ampere = Unit<BaseTypes::CURRENT>;
using Candela = Unit<BaseTypes::LUMINOUS_INTENSITY>;

// Standard SI units
using Hertz = UnitInverse<Second>;
using Newton = MultiUnit<Kilogram, Meter, Hertz, Hertz>;
using Pascal = MultiUnit<Kilogram, UnitInverse<Meter>, Hertz, Hertz>;
using Joule = MultiUnit<Newton, Meter>;
using Watt = MultiUnit<Joule, Hertz>;
using Coulomb = MultiUnit<Second, Ampere>;
using Volt = MultiUnit<Watt, UnitInverse<Ampere>>;
using Farad = MultiUnit<Coulomb, UnitInverse<Volt>>;
using Ohm = MultiUnit<Volt, UnitInverse<Ampere>>;
using Siemens = UnitInverse<Ohm>;
using Weber = MultiUnit<Volt, Second>;
using Tesla = MultiUnit<Weber, UnitInverse<Meter>, UnitInverse<Meter>>;
using Henry = MultiUnit<Weber, UnitInverse<Ampere>>;
using Lux = MultiUnit<Candela, UnitInverse<Meter>, UnitInverse<Meter>>;
using Becquerel = Hertz;
using Gray = MultiUnit<Joule, UnitInverse<Kilogram>>;
using Sievert = Gray;

// "Nonstandard" SI units
using Minute = UnitRatio<Second, std::ratio<60, 1>>;
using Hour = UnitRatio<Minute, std::ratio<60, 1>>;
using Day = UnitRatio<Hour, std::ratio<24, 1>>;
using AstronomicalUnit = UnitRatio<Meter, std::ratio<149597870700, 1>>;
using Hectare = UnitRatio<MultiUnit<Meter, Meter>, std::ratio<10000, 1>>;
using Liter = UnitRatio<MultiUnit<Meter, Meter, Meter>, std::ratio<1, 1000>>;
using Litre = Liter;
using Tonne = UnitRatio<Kilogram, std::ratio<1000, 1>>;
using MetricTon = Tonne;

// FPS units in terms of SI units
using Foot = UnitRatio<Meter, std::ratio<3048, 10000>>;
using Yard = UnitRatio<Foot, std::ratio<3, 1>>;
using Mile = UnitRatio<Foot, std::ratio<5280, 1>>;
using Inch = UnitRatio<Foot, std::ratio<1, 12>>;
using Slug = UnitRatio<Kilogram, std::ratio<145939, 10000>>;
using Pound = MultiUnit<Slug, Foot, Hertz, Hertz>;
using Kip = UnitRatio<Pound, std::kilo>;
using FootPound = MultiUnit<Foot, Pound>;
using PSI = MultiUnit<Pound, UnitInverse<Inch>, UnitInverse<Inch>>;

// Other Units
using Gram = UnitRatio<Kilogram, std::milli>;
using Atmosphere = UnitRatio<Pascal, std::ratio<101325, 1>>;
using Torr = UnitRatio<Atmosphere, std::ratio<1, 760>>;
using mmHg = Torr;
using mps = MultiUnit<Meter, Hertz>;
using mph = MultiUnit<Mile, UnitInverse<Hour>>;
using Rankine = UnitRatio<Kelvin, std::ratio<10, 18>>;
using Celsius = UnitOffset<Kelvin, std::ratio<27315, 100>>;
using Fahrenheit = UnitOffset<Rankine, std::ratio<45967, 100>>;

// Helpful templates
template<UnitType T>
using Square = MultiUnit<T, T>;
template<UnitType T>
using Cubic = MultiUnit<T, T, T>;
template<UnitType T>
using Quartic = MultiUnit<T, T, T, T>;
template<UnitType T>
using Per = UnitInverse<T>; // redundant, but more in line with english; eg. "per meter"
template<UnitType T>
using Milli = UnitRatio<T, std::milli>;
template<UnitType T>
using Centi = UnitRatio<T, std::centi>;
template<UnitType T>
using Deci = UnitRatio<T, std::deci>;
template<UnitType T>
using Deca = UnitRatio<T, std::deca>;
template<UnitType T>
using Hecto = UnitRatio<T, std::hecto>;
template<UnitType T>
using Kilo = UnitRatio<T, std::kilo>;
template<UnitType T>
using Mega = UnitRatio<T, std::mega>;
template<UnitType T>
using Giga = UnitRatio<T, std::giga>;
template<UnitType T>
using Tera = UnitRatio<T, std::tera>;

template<UnitType T1, UnitType T2>
constexpr auto getConversionFactor(const double factor) {
    return const_cast<const MultiUnit<T1, Per<T2>>>(MultiUnit<T1, Per<T2>>{factor});
}

#endif //UNITMAKER_SI_UNITS_H
