#include "core.hpp"

namespace papilio
{
template <typename FormatContext>
void format_arg::format(format_parse_context& parse_ctx, FormatContext& out_ctx) const
{
    visit(
        [&]<typename T>(const T& v)
        {
            if constexpr(std::is_same_v<T, handle>)
            {
                dynamic_format_context dyn_ctx(out_ctx);
                v.format(parse_ctx, dyn_ctx);
            }
            else if constexpr(formattable<T>)
            {
                using formatter_t = PAPILIO_NS formatter<T, char_type>;
                formatter_t fmt;
                parse_ctx.advance_to(fmt.parse(parse_ctx));

                using context_t = format_context_traits<FormatContext>;
                context_t::advance_to(out_ctx, fmt.format(v, out_ctx));
            }
        }
    );
}
} // namespace papilio
