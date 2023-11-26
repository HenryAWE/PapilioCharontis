#include <papilio/script/variable.hpp>


namespace papilio::script
{
    std::partial_ordering variable::compare(const variable& var) const
    {
        auto visitor = [](auto&& lhs, auto&& rhs)->std::partial_ordering
        {
            using std::is_same_v;
            using T = std::remove_cvref_t<decltype(lhs)>;
            using U = std::remove_cvref_t<decltype(rhs)>;

            if constexpr(is_same_v<T, U>)
            {
                return lhs <=> rhs;
            }
            else if constexpr(is_same_v<T, utf::string_container> || is_same_v<U, utf::string_container>)
            {
                throw std::invalid_argument("invalid argument");
            }
            else
            {
                using R = std::common_type_t<T, U>;
                return static_cast<R>(lhs) <=> static_cast<R>(rhs);
            }
        };

        return std::visit(visitor, m_var, var.to_underlying());
    }

    bool variable::equal(
        const variable& var,
        float_type epsilon
    ) const noexcept {
        auto visitor = [epsilon](auto&& lhs, auto&& rhs)->bool
        {
            using std::is_same_v;
            using T = std::remove_cvref_t<decltype(lhs)>;
            using U = std::remove_cvref_t<decltype(rhs)>;

            if constexpr(is_same_v<T, U>)
            {
                if constexpr(std::floating_point<T>)
                {
                    return std::abs(lhs - rhs) < epsilon;
                }
                else
                {
                    return lhs == rhs;
                }
            }
            else
            {
                if constexpr(is_same_v<T, utf::string_container> || is_same_v<U, utf::string_container>)
                {
                    return false;
                }
                else
                {
                    using R = std::common_type_t<T, U>;
                    if constexpr(std::floating_point<R>)
                    {
                        return std::abs(static_cast<R>(lhs) - static_cast<R>(rhs)) < epsilon;
                    }
                    else
                    {
                        return static_cast<R>(lhs) == static_cast<R>(rhs);
                    }
                }
            }
        };

        return std::visit(visitor, m_var, var.to_underlying());
    }
}
