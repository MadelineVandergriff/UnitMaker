//
// Copyright (c) 2020 Jack Vandergriff.
//

#ifndef UNITMAKER_UNITS_H
#define UNITMAKER_UNITS_H

#define UNIT_SET_RATIO(type, numerator, denominator) template<> intmax_t type::ratio::num = ( numerator ); template<> intmax_t type::ratio::den = ( denominator )

#include <ratio>
#include <concepts>
#include <type_traits>

template<typename, typename = void>
struct is_ratio : std::false_type {};

template<typename T>
struct is_ratio<T, std::void_t<decltype(T::num), decltype(T::den)>> :
        std::conjunction<std::is_convertible<decltype(T::num), std::intmax_t>, std::is_convertible<decltype(T::den), std::intmax_t>> {};

template<typename T>
inline constexpr bool is_ratio_v = is_ratio<T>::value;

template<typename, typename = void>
struct is_unit : std::false_type {};

template<typename T>
struct is_unit<T, std::void_t<typename T::base_type, typename T::ratio, decltype(T::value)>> :
        std::conjunction<is_ratio<typename T::base_type>, is_ratio<typename T::ratio>, std::is_convertible<decltype(T::value), double>> {};

template<typename T>
inline constexpr bool is_unit_v = is_unit<T>::value;

template<typename, typename, typename = void>
struct has_equivalent_base_type : std::false_type {};

template<typename T1, typename T2>
struct has_equivalent_base_type<T1, T2, std::enable_if_t<is_unit_v<T1> && is_unit_v<T2>, void>> :
        std::ratio_equal<typename T1::base_type, typename T2::base_type> {};

template<typename T1, typename T2>
inline constexpr bool has_equivalent_base_type_v = has_equivalent_base_type<T1, T2>::value;

enum class BaseTypes {
    MASS=2, LENGTH=3, TIME=5, TEMPERATURE=7, CURRENT=11, LUMINOUS_INTENSITY=13
};

template<typename T>
struct RuntimeRatio {
    static inline intmax_t num = 1;
    static inline intmax_t den = 1;
    using type = T;
};

template<typename T=std::ratio<1, 1>, typename ...Ts>
struct RecursiveRatioMultiply {
    static_assert(is_ratio_v<T>, "RecursiveRatioMultiply only accepts valid ratio types (see is_ratio<T>)");
    using ratio = std::ratio_multiply<T, typename RecursiveRatioMultiply<Ts...>::ratio>;
};

template<>
struct RecursiveRatioMultiply<std::ratio<1, 1>> {
    using ratio = std::ratio<1, 1>;
};

template<typename T>
struct AbstractUnit {
private:
    template<typename To, typename From, class = typename
            std::enable_if_t<has_equivalent_base_type_v<To, From>>>
    static constexpr To convert(const From& from) {
        constexpr double ratio = 1.0 * From::ratio::num * To::ratio::den / From::ratio::den / To::ratio::num;
        return To{from.value * ratio};
    }

    template<typename To, typename From, class = typename
            std::enable_if_t<is_unit_v<From> && std::ratio_equal_v<typename From::ratio, std::ratio<1, 1>> && std::is_convertible_v<double, To>>>
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


template<BaseTypes Type>
struct Unit : public AbstractUnit<Unit<Type>> {
    using AbstractUnit<Unit<Type>>::AbstractUnit;

    using base_type = std::ratio<(int)Type, 1>;
    using ratio = std::ratio<1, 1>;
};

template<typename T, typename Ratio>
struct UnitRatio : public AbstractUnit<UnitRatio<T, Ratio>> {
    static_assert(is_unit_v<T>, "UnitRatio must be passed a valid unit (see is_unit<T>)");
    static_assert(is_ratio_v<Ratio>, "UnitRatio must be passed a valid ratio (see is_ratio<T>)");

    using AbstractUnit<UnitRatio<T, Ratio>>::AbstractUnit;

    using base_type = typename T::base_type;
    using ratio = std::ratio_multiply<Ratio, typename T::ratio>;
};

template<typename ...Ts>
struct MultiUnit : public AbstractUnit<MultiUnit<Ts...>> {
    static_assert(std::conjunction_v<is_unit<Ts>...>, "MultiUnit must be passed only valid units (see is_unit<T>)");

    using AbstractUnit<MultiUnit<Ts...>>::AbstractUnit;

    using base_type = typename RecursiveRatioMultiply<typename Ts::base_type...>::ratio;
    using ratio = typename RecursiveRatioMultiply<typename Ts::ratio...>::ratio;
};

template<typename BaseType, typename Ratio=std::ratio<1, 1>>
struct SpecifiedUnit : public AbstractUnit<SpecifiedUnit<BaseType, Ratio>> {
    static_assert(is_ratio_v<BaseType>, "SpecifiedUnit requires a ratio as BaseType (see is_ratio<T>)");
    static_assert(is_ratio_v<Ratio>, "SpecifiedUnit requires a ratio as Ratio (see is_ratio<T>)");

    using AbstractUnit<SpecifiedUnit<BaseType, Ratio>>::AbstractUnit;

    using base_type = BaseType;
    using ratio = Ratio;
};

template<BaseTypes Type>
struct RuntimeUnit {
    double value;
    explicit constexpr RuntimeUnit(double v): value(v) {}

    template<typename T, class = typename std::enable_if_t<has_equivalent_base_type_v<T, RuntimeUnit<Type>>>>
    explicit constexpr RuntimeUnit(T other): value{Unit<Type>{other}.value / ratio::num * ratio::den} {}

    template<typename T, class = typename std::enable_if_t<has_equivalent_base_type_v<T, RuntimeUnit<Type>>>>
    constexpr operator T() {
        return T{Unit<Type>{value * ratio::num / ratio::den}};
    }

    using base_type = std::ratio<(int) Type, 1>;
    struct ratio {
        static inline intmax_t num = 1;
        static inline intmax_t den = 1;
    };
};

template<typename T>
struct UnitInverse : public AbstractUnit<UnitInverse<T>> {
    static_assert(is_unit_v<T>, "UnitInverse must be passed a valid unit (see is_unit<T>)");

    using AbstractUnit<UnitInverse<T>>::AbstractUnit;

    using base_type = std::ratio_divide<std::ratio<1, 1>, typename T::base_type>;
    using ratio = std::ratio_divide<std::ratio<1, 1>, typename T::ratio>;
};

template<typename T, typename Offset>
struct UnitOffset {
    static_assert(is_unit_v<T>, "UnitOffset must be passed a valid unit (see is_unit<T>)");
    static_assert(is_ratio_v<Offset>, "UnitOffset must be passed a valid ratio (see is_ratio<T>)");

    double value;

    explicit constexpr UnitOffset(double d): value{d} {}

    //using base_type = typename T::base_type;
    //using ratio = typename T::ratio;

    template<typename To, class = typename std::enable_if_t<has_equivalent_base_type_v<T, To>>>
    constexpr operator T() const {
        constexpr double offset = 1.0 * Offset::num / Offset::den;
        return To{T{value + offset}};
    }
};

template<typename T1, typename T2, class = typename
        std::enable_if_t<is_unit_v<T1> && is_unit_v<T2>>>
constexpr MultiUnit<T1, T2> operator*(const T1& t1, const T2& t2) {
    return MultiUnit<T1, T2>{t1.value * t2.value};
}

template<typename T1, typename T2, class = typename
        std::enable_if_t<std::is_arithmetic_v<T1> && is_unit_v<T2>>>
constexpr T2 operator*(const T1& v, const T2& t) {
    return T2{t.value * v};
}

template<typename T1, typename T2, class = typename
        std::enable_if_t<std::is_arithmetic_v<T2> && is_unit_v<T1>>>
constexpr T1 operator*(const T1& t, const T2& v) {
    return T1{t.value * v};
}

template<typename T1, typename T2, class = typename
        std::enable_if_t<is_unit_v<T1> && is_unit_v<T2>>>
constexpr MultiUnit<T1, UnitInverse<T2>> operator/(const T1& t1, const T2& t2) {
    return MultiUnit<T1, UnitInverse<T2>>{t1.value / t2.value};
}

template<typename T1, typename T2, class = typename
        std::enable_if_t<std::is_arithmetic_v<T1> && is_unit_v<T2>>>
constexpr UnitInverse<T2> operator/(const T1& v, const T2& t) {
    return UnitInverse<T2>{v / t.value};
}

template<typename T1, typename T2, class = typename
        std::enable_if_t<std::is_arithmetic_v<T2> && is_unit_v<T1>>>
constexpr T1 operator/(const T1& t, const T2& v) {
    return T1{t.value / v};
}

template<typename T1, typename T2, class = typename
        std::enable_if_t<has_equivalent_base_type_v<T1, T2>>>
constexpr T1 operator+(const T1& t1, const T2& t2) {
    return T1{t1.value + T1{t2}.value};
}

template<typename T1, typename T2, class = typename
        std::enable_if_t<has_equivalent_base_type_v<T1, T2>>>
constexpr T1 operator-(const T1& t1, const T2& t2) {
    return T1{t1.value - T1{t2}.value};
}

#endif //UNITMAKER_UNITS_H
