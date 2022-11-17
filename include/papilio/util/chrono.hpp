#pragma once

#include <string>
#include <chrono>
#include <ctime>
#include "../core.hpp"


namespace papilio
{
    template <>
    class formatter<std::tm>
    {
    public:
        void parse(format_spec_parse_context& ctx)
        {
            m_fmt = std::string_view(ctx);
        }
        template <typename Context>
        void format(const std::tm& val, Context& ctx)
        {
            using context_traits = format_context_traits<Context>;

            if(m_fmt.empty())
            {
                context_traits::append(ctx, std::asctime(&val));
            }
            else
            {
                char buf[128];
                std::size_t len = std::strftime(buf, 128, m_fmt.c_str(), &val);
                context_traits::append(ctx, buf, buf + len);
            }
        }

    private:
        std::string m_fmt;
    };
}
