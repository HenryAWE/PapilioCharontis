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
            if(!m_fmt.empty())
                m_fmt.make_independent();
        }
        template <typename Context>
        void format(const std::tm& val, Context& ctx)
        {
            format_context_traits traits(ctx);
            if(m_fmt.empty())
            {
                traits.append(std::asctime(&val));
            }
            else
            {
                char buf[128];
                std::size_t len = std::strftime(buf, 128, m_fmt.c_str(), &val);
                traits.append(buf, buf + len);
            }
        }

    private:
        string_container m_fmt;
    };
}
