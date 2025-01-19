#ifndef PAPILIO_FORMATTER_CONTAINER_ADAPTOR_HPP
#define PAPILIO_FORMATTER_CONTAINER_ADAPTOR_HPP

#include <stack>
#include <queue>
#include "../format.hpp"
#include "../detail/prefix.hpp"

namespace papilio
{
template <container_adaptor Adaptor, typename CharT = char>
class container_adaptor_formatter :
    public range_formatter<typename Adaptor::container_type, CharT>
{
    using my_base = range_formatter<typename Adaptor::container_type, CharT>;

public:
    template <typename ParseContext, typename FormatContext>
    auto format(const Adaptor& adaptor, ParseContext& parse_ctx, FormatContext& fmt_ctx) const
    {
        return my_base::format(
            adaptor_extractor<Adaptor>::get(adaptor),
            parse_ctx,
            fmt_ctx
        );
    }
};

template <typename T, typename Container, typename CharT>
class formatter<std::stack<T, Container>, CharT> :
    public container_adaptor_formatter<std::stack<T, Container>, CharT>
{};

template <typename T, typename Container, typename CharT>
class formatter<std::queue<T, Container>, CharT> :
    public container_adaptor_formatter<std::queue<T, Container>, CharT>
{};

template <typename T, typename Container, typename Compare, typename CharT>
class formatter<std::priority_queue<T, Container, Compare>, CharT> :
    public container_adaptor_formatter<std::priority_queue<T, Container, Compare>, CharT>
{};
} // namespace papilio

#include "../detail/suffix.hpp"

#endif
