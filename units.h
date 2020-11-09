//
// Copyright (c) 2020 Jack Vandergriff.
//

#ifndef UNITMAKER_UNITS_H
#define UNITMAKER_UNITS_H

#define UNIT_SET_RATIO(type, numerator, denominator) template<> intmax_t type::ratio::num = ( numerator ); template<> intmax_t type::ratio::den = ( denominator )

#include <ratio>
#include <concepts>
#include <type_traits>

template<typename T>
concept RatioType = requires (){
    {T::num} -> std::convertible_to<std::intmax_t>;
    {T::den} -> std::convertible_to<std::intmax_t>;
};

template<typename T>
concept UnitType =
    RatioType<typename T::base_type> &&
    RatioType<typename T::ratio> &&
    std::is_arithmetic_v<decltype(T::value)>;

template<typename T1, typename T2>
concept EquivalentBaseType =
    UnitType<T1> &&
    UnitType<T2> &&
    std::ratio_equal_v<typename T1::base_type, typename T2::base_type>;

enum class BaseTypes {
    MASS=2, LENGTH=3, TIME=5, TEMPERATURE=7, CURRENT=11, LUMINOUS_INTENSITY=13
};

template<typename T>
struct RuntimeRatio {
    static inline intmax_t num = 1;
    static inline intmax_t den = 1;
    using type = T;
};

template<RatioType T=std::ratio<1, 1>, RatioType ...Ts>
struct RecursiveRatioMultiply {
    using ratio = std::ratio_multiply<T, typename RecursiveRatioMultiply<Ts...>::ratio>;
};

template<>
struct RecursiveRatioMultiply<std::ratio<1, 1>> {
    using ratio = std::ratio<1, 1>;
};

template<typename T, typename Numeric = double> // can't constrain on UnitType, since type will be incomplete at this point
requires std::is_arithmetic_v<Numeric>
struct AbstractUnit {
private:
    template<UnitType To, UnitType From>
    requires EquivalentBaseType<To, From>
    static constexpr To convert(const From& from) {
        using ratio_t = std::ratio<From::ratio::num * To::ratio::den, From::ratio::den * To::ratio::num>;
        if constexpr (std::is_integral_v<decltype(To::value)> && ratio_t::den == 1) {
            return To{static_cast<decltype(To::value)>(from.value * ratio_t::num)};
        } else {
            constexpr double ratio = 1.0 * ratio_t::num / ratio_t::den;
            return To{from.value * ratio};
        }
    }

    template<typename To, UnitType From>
    requires std::ratio_equal_v<typename From::ratio, std::ratio<1, 1>> && std::convertible_to<Numeric, To>
    static constexpr To convert(const From& from) {
        return from.value;
    }
public:
    Numeric value;
    explicit constexpr AbstractUnit(Numeric v) : value{v} {}

    template<typename To>
    constexpr operator To() const {
        return convert<To, T>(static_cast<const T&>(*this));
    }
};


template<BaseTypes Type, typename Numeric = double>
struct Unit : public AbstractUnit<Unit<Type, Numeric>, Numeric> {
    using AbstractUnit<Unit<Type, Numeric>, Numeric>::AbstractUnit;

    using base_type = std::ratio<(int)Type, 1>;
    using ratio = std::ratio<1, 1>;
};

template<UnitType T, RatioType Ratio>
struct UnitRatio : public AbstractUnit<UnitRatio<T, Ratio>, decltype(T::value)> {
    using AbstractUnit<UnitRatio<T, Ratio>, decltype(T::value)>::AbstractUnit;

    using base_type = typename T::base_type;
    using ratio = std::ratio_multiply<Ratio, typename T::ratio>;
};

template<UnitType ...Ts>
struct MultiUnit : public AbstractUnit<MultiUnit<Ts...>, std::common_type_t<decltype(Ts::value)...>> {
    using AbstractUnit<MultiUnit<Ts...>, std::common_type_t<decltype(Ts::value)...>>::AbstractUnit;

    using base_type = typename RecursiveRatioMultiply<typename Ts::base_type...>::ratio;
    using ratio = typename RecursiveRatioMultiply<typename Ts::ratio...>::ratio;
};

template<RatioType BaseType, RatioType Ratio=std::ratio<1, 1>, typename Numeric = double>
struct SpecifiedUnit : public AbstractUnit<SpecifiedUnit<BaseType, Ratio, Numeric>, Numeric> {
    using AbstractUnit<SpecifiedUnit<BaseType, Ratio, Numeric>, Numeric>::AbstractUnit;

    using base_type = BaseType;
    using ratio = Ratio;
};

template<UnitType Type, typename Numeric>
using NumericUnit = SpecifiedUnit<typename Type::base_type, typename Type::ratio, Numeric>;

template<BaseTypes Type, int ID, typename Numeric = double>
struct RuntimeUnit {
    Numeric value;
    explicit constexpr RuntimeUnit(Numeric v): value(v) {}

    template<UnitType T>
    requires EquivalentBaseType<T, RuntimeUnit<Type, ID>>
    explicit constexpr RuntimeUnit(T other): value{Unit<Type>{other}.value / ratio::num * ratio::den} {}

    template<UnitType T>
    requires EquivalentBaseType<RuntimeUnit<Type, ID>, T>
    constexpr operator T() {
        return T{Unit<Type>{value * ratio::num / ratio::den}};
    }

    using base_type = std::ratio<(int) Type, 1>;
    struct ratio {
        static inline intmax_t num = 1;
        static inline intmax_t den = 1;
    };
};

template<UnitType T>
struct UnitInverse : public AbstractUnit<UnitInverse<T>, decltype(T::value)> {
    using AbstractUnit<UnitInverse<T>, decltype(T::value)>::AbstractUnit;

    using base_type = std::ratio_divide<std::ratio<1, 1>, typename T::base_type>;
    using ratio = std::ratio_divide<std::ratio<1, 1>, typename T::ratio>;
};

template<UnitType T, RatioType Offset, typename Numeric = double>
struct UnitOffset {
    Numeric value;

    explicit constexpr UnitOffset(Numeric d): value{d} {}

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
requires std::is_arithmetic_v<T1> && UnitType<T2>
constexpr auto operator*(const T1& v, const T2& t) {
    return SpecifiedUnit<typename T2::base_type, typename T2::ratio, decltype(t.value * v)>{t.value * v};
}

template<typename T1, typename T2>
requires std::is_arithmetic_v<T2> && UnitType<T1>
constexpr auto operator*(const T1& t, const T2& v) {
    return SpecifiedUnit<typename T1::base_type, typename T1::ratio, decltype(t.value * v)>{t.value * v};
}

template<UnitType T1, UnitType T2>
constexpr MultiUnit<T1, UnitInverse<T2>> operator/(const T1& t1, const T2& t2) {
    return MultiUnit<T1, UnitInverse<T2>>{t1.value / t2.value};
}

template<typename T1, typename T2>
requires std::is_arithmetic_v<T1> && UnitType<T2>
constexpr auto operator/(const T1& v, const T2& t) {
    return UnitInverse<SpecifiedUnit<typename T2::base_type, typename T2::ratio, decltype(v / t.value)>>{v / t.value};
}

template<typename T1, typename T2>
requires std::is_arithmetic_v<T2> && UnitType<T1>
constexpr auto operator/(const T1& t, const T2& v) {
    return SpecifiedUnit<typename T1::base_type, typename T1::ratio, decltype(t.value / v)>{t.value / v};
}

template<UnitType T1, UnitType T2>
requires EquivalentBaseType<T1, T2>
constexpr auto operator+(const T1& t1, const T2& t2) {
    using ret_type = SpecifiedUnit<typename T1::base_type, typename T1::ratio,
            std::conditional_t<std::ratio_divide<typename T2::ratio, typename T1::ratio>::den == 1, decltype(t1.value + t2.value), decltype(t1.value + 1.0 * t2.value)>>;
    return ret_type{t1.value + ret_type{t2}.value};
}

template<UnitType T1, UnitType T2>
requires EquivalentBaseType<T1, T2>
constexpr auto operator-(const T1& t1, const T2& t2) {
    using ret_type = SpecifiedUnit<typename T1::base_type, typename T1::ratio,
            std::conditional_t<std::ratio_divide<typename T2::ratio, typename T1::ratio>::den == 1, decltype(t1.value - t2.value), decltype(t1.value - 1.0 * t2.value)>>;
    return ret_type{t1.value - ret_type{t2}.value};
}

#endif //UNITMAKER_UNITS_H
