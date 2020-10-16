//
// Copyright (c) 2020 Jack Vandergriff.
//

#ifndef UNITMAKER_UNITS_H
#define UNITMAKER_UNITS_H

#include <ratio>
#include <concepts>
#include <type_traits>

template<typename T>
concept RatioType = requires (){
    {T::num} -> std::convertible_to<std::intmax_t>;
    {T::den} -> std::convertible_to<std::intmax_t>;
};

template<typename T>
concept UnitType = requires (T t){
    RatioType<typename T::base_type>;
    RatioType<typename T::ratio>;
    {t.value} -> std::convertible_to<double>;
};

template<typename T1, typename T2>
concept EquivalentBaseType =
    UnitType<T1> &&
    UnitType<T2> &&
    std::ratio_equal_v<typename T1::base_type, typename T2::base_type>;

enum class BaseTypes {
    MASS=2, LENGTH=3, TIME=5, TEMPERATURE=7, CURRENT=11, LUMINOUS_INTENSITY=13
};

template<RatioType T=std::ratio<1, 1>, RatioType ...Ts>
struct RecursiveRatioMultiply {
    using ratio = std::ratio_multiply<T, typename RecursiveRatioMultiply<Ts...>::ratio>;
};

template<>
struct RecursiveRatioMultiply<std::ratio<1, 1>> {
    using ratio = std::ratio<1, 1>;
};

template<typename T> // can't constrain on UnitType, since type will be incomplete at this point
struct AbstractUnit {
private:
    template<UnitType To, UnitType From>
    requires EquivalentBaseType<To, From>
    static constexpr To convert(const From& from) {
        constexpr double ratio = 1.0 * From::ratio::num * To::ratio::den / From::ratio::den / To::ratio::num;
        return To{from.value * ratio};
    }

    template<typename To, UnitType From>
    requires std::ratio_equal_v<typename From::ratio, std::ratio<1, 1>> && std::convertible_to<double, To>
    static constexpr double convert(const From& from) {
        return from.value;
    }
public:
    double value;
    explicit constexpr AbstractUnit(double v) : value{v} {}

    template<typename To>
    constexpr operator To() const {
        return convert<To, T>(static_cast<const T&>(*this));
    }
};

template<UnitType T, RatioType Ratio>
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

template<UnitType ...Ts>
struct MultiUnit : public AbstractUnit<MultiUnit<Ts...>> {
    using AbstractUnit<MultiUnit<Ts...>>::AbstractUnit;

    using base_type = typename RecursiveRatioMultiply<typename Ts::base_type...>::ratio;
    using ratio = typename RecursiveRatioMultiply<typename Ts::ratio...>::ratio;
};

template<RatioType BaseType, RatioType Ratio=std::ratio<1, 1>>
struct SpecifiedUnit : public AbstractUnit<SpecifiedUnit<BaseType, Ratio>> {
    using AbstractUnit<SpecifiedUnit<BaseType, Ratio>>::AbstractUnit;

    using base_type = BaseType;
    using ratio = Ratio;
};

template<UnitType T>
struct UnitInverse : public AbstractUnit<UnitInverse<T>> {
    using AbstractUnit<UnitInverse<T>>::AbstractUnit;

    using base_type = std::ratio_divide<std::ratio<1, 1>, typename T::base_type>;
    using ratio = std::ratio_divide<std::ratio<1, 1>, typename T::ratio>;
};

template<UnitType T, RatioType Offset>
struct UnitOffset {
    double value;

    explicit constexpr UnitOffset(double d): value{d} {}

    //using base_type = typename T::base_type;
    //using ratio = typename T::ratio;

    template<UnitType To>
    requires EquivalentBaseType<T, To>
    constexpr operator To() const {
        constexpr double offset = 1.0 * Offset::num / Offset::den;
        return To{T{value + offset}};
    }
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

template<UnitType T1, UnitType T2>
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
    return T1{t.value / v};
}

template<UnitType T1, UnitType T2>
requires EquivalentBaseType<T1, T2>
constexpr T1 operator+(const T1& t1, const T2& t2) {
    return T1{t1.value + T1{t2}.value};
}

template<UnitType T1, UnitType T2>
requires EquivalentBaseType<T1, T2>
constexpr T1 operator-(const T1& t1, const T2& t2) {
    return T1{t1.value - T1{t2}.value};
}

#endif //UNITMAKER_UNITS_H
