//
// Copyright (c) 2020 Jack Vandergriff.
//

#ifndef UNITMAKER_UNITS_H
#define UNITMAKER_UNITS_H

#include <ratio>
#include <type_traits>

template<typename T>
concept UnitType = requires (){
    {std::ratio_multiply<typename T::base_type, typename T::ratio>::den} -> std::convertible_to<std::intmax_t>;
};

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

template<typename T>
struct AbstractUnit {
private:
    template<typename To, typename From, class = typename std::enable_if_t<std::ratio_equal_v<typename To::base_type, typename From::base_type>>>
    static constexpr To convert(const From& from) {
        constexpr double ratio = 1.0 * From::ratio::num * To::ratio::den / From::ratio::den / To::ratio::num;
        return To{from.value * ratio};
    }
public:
    double value;
    explicit constexpr AbstractUnit(double v) : value{v} {}

    template<typename To>
    constexpr operator To() const {
        return convert<To>(static_cast<const T&>(*this));
    }
};

template<typename T, typename Ratio>
struct UnitRatio : public AbstractUnit<UnitRatio<T, Ratio>> {
    using AbstractUnit<UnitRatio<T, Ratio>>::AbstractUnit;

    using base_type = typename T::base_type;
    using ratio = std::ratio_multiply<Ratio, typename T::ratio>;
};

template<BaseTypes Type>
struct Unit : public AbstractUnit<Unit<Type>> {
    using AbstractUnit<Unit<Type>>::AbstractUnit;

    using base_type = std::ratio<(int)Type, 1>;
    using ratio = std::ratio<1, 1>;
};

template<typename ...Ts>
struct MultiUnit : public AbstractUnit<MultiUnit<Ts...>> {
    using AbstractUnit<MultiUnit<Ts...>>::AbstractUnit;

    using base_type = typename RecursiveRatioMultiply<typename Ts::base_type...>::ratio;
    using ratio = typename RecursiveRatioMultiply<typename Ts::ratio...>::ratio;
};

template<typename T>
struct UnitInverse : public AbstractUnit<UnitInverse<T>> {
    using AbstractUnit<UnitInverse<T>>::AbstractUnit;

    using base_type = std::ratio_divide<std::ratio<1, 1>, typename T::base_type>;
    using ratio = std::ratio_divide<std::ratio<1, 1>, typename T::ratio>;
};

template<typename T, typename Offset>
struct UnitOffset {
    double value;
};

template<UnitType T1, UnitType T2>
constexpr MultiUnit<T1, T2> operator*(const T1& t1, const T2& t2) {
    return MultiUnit<T1, T2>{t1.value * t2.value};
}

template<typename T1, typename T2>
requires std::is_floating_point_v<T1> && UnitType<T2>
constexpr T2 operator*(const T1& v, const T2& t) {
    return T2{t.value * v};
}

template<typename T1, typename T2>
requires std::is_floating_point_v<T2> && UnitType<T1>
constexpr T1 operator*(const T1& t, const T2& v) {
    return T2{t.value * v};
}

template<typename T1, typename T2>
constexpr MultiUnit<T1, UnitInverse<T2>> operator/(const T1& t1, const T2& t2) {
    return MultiUnit<T1, UnitInverse<T2>>{t1.value / t2.value};
}

template<typename T1, typename T2>
requires std::is_floating_point_v<T1> && UnitType<T2>
constexpr UnitInverse<T2> operator/(const T1& v, const T2& t) {
    return UnitInverse<T2>{v / t.value};
}

template<typename T1, typename T2>
requires std::is_floating_point_v<T2> && UnitType<T1>
constexpr T1 operator/(const T1& t, const T2& v) {
    return T2{t.value / v};
}

#endif //UNITMAKER_UNITS_H
