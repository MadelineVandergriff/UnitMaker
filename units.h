//
// Copyright (c) 2020 Jack Vandergriff.
//

#ifndef UNITMAKER_UNITS_H
#define UNITMAKER_UNITS_H

#include <ratio>
#include <type_traits>

enum class BaseTypes {
    MASS=2, LENGTH=3, TIME=5, TEMPERATURE=7, CURRENT=11, LUMINOUS_INTENSITY=13
};

template<typename T=std::ratio<1, 1>, typename ...Ts>
struct RecursiveRatioMultiply {
    using ratio = std::ratio_multiply<T, typename RecursiveRatioMultiply<Ts...>::ratio>;
};

template<>
struct RecursiveRatioMultiply<std::ratio<1, 1>> {
    using ratio = std::ratio<1, 1>;
};

template<typename T, typename Ratio>
struct UnitRatio {
    double value;
    explicit constexpr UnitRatio(double v) : value{v} {}

    template<typename To>
    constexpr operator To() const {
        return convert<To>(*this);
    }

    using base_type = typename T::base_type;
    using ratio = std::ratio_multiply<Ratio, typename T::ratio>;
};

template<BaseTypes Type>
struct Unit {
    double value;
    explicit constexpr Unit(double v) : value{v} {}

    template<typename To>
    constexpr operator To() const {
        return convert<To>(*this);
    }

    using base_type = std::ratio<(int)Type, 1>;
    using ratio = std::ratio<1, 1>;
};

template<typename ...Ts>
struct MultiUnit {
    double value;
    explicit constexpr MultiUnit(double v) : value{v} {}

    template<typename To>
    constexpr operator To() const {
        return convert<To>(*this);
    }

    using base_type = typename RecursiveRatioMultiply<typename Ts::base_type...>::ratio;
    using ratio = typename RecursiveRatioMultiply<typename Ts::ratio...>::ratio;
};

template<typename T>
struct UnitInverse {
    double value;
    explicit constexpr UnitInverse(double v) : value{v} {}

    template<typename To>
    constexpr operator To() const {
        return convert<To>(*this);
    }

    using base_type = std::ratio_divide<std::ratio<1, 1>, typename T::base_type>;
    using ratio = std::ratio_divide<std::ratio<1, 1>, typename T::ratio>;
};

template<typename T, typename Offset>
struct UnitOffset {
    double value;
};

template<typename To, typename From, class = typename std::enable_if_t<std::ratio_equal_v<typename To::base_type, typename From::base_type>>>
constexpr To convert(const From& from) {
    return To{from.value / From::ratio::den * From::ratio::num / To::ratio::num * To::ratio::den};
}

template<typename T1, typename T2>
constexpr MultiUnit<T1, T2> operator*(const T1& t1, const T2& t2) {
    return MultiUnit<T1, T2>{t1.value * t2.value};
}

template<typename T1, typename T2>
constexpr MultiUnit<T1, UnitInverse<T2>> operator/(const T1& t1, const T2& t2) {
    return MultiUnit<T1, UnitInverse<T2>>{t1.value / t2.value};
}

#endif //UNITMAKER_UNITS_H
