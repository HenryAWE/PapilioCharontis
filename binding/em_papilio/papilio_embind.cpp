#include <emscripten/bind.h>
#include <papilio/papilio.hpp>

namespace em_papilio
{
class context
{
public:
    void push(emscripten::val val)
    {
        if(val.isTrue())
        {
            m_args.push(true);
        }
        else if(val.isFalse())
        {
            m_args.push(false);
        }
        else if(val.isNumber())
        {
            m_args.push(val.as<float>());
        }
        else if(val.isString())
        {
            m_args.push(val.as<std::string>());
        }
        else
        {
            m_args.push(papilio::format_arg());
        }
    }

    std::string format(const std::string& fmt)
    {
        return papilio::vformat(fmt, m_args);
    }

private:
    papilio::dynamic_format_args m_args;
};

static std::string vformat_impl(const std::string& fmt, std::initializer_list<emscripten::val> vals)
{
    context ctx;
    for(auto&& i : vals)
        ctx.push(i);
    return ctx.format(fmt);
}

// Plain text
std::string format_impl_0(const std::string& fmt)
{
    return vformat_impl(fmt, {});
}

std::string format_impl_1(
    const std::string& fmt,
    emscripten::val v1
)
{
    return vformat_impl(fmt, {v1});
}

std::string format_impl_2(
    const std::string& fmt,
    emscripten::val v1,
    emscripten::val v2
)
{
    return vformat_impl(fmt, {v1, v2});
}

std::string format_impl_3(
    const std::string& fmt,
    emscripten::val v1,
    emscripten::val v2,
    emscripten::val v3
)
{
    return vformat_impl(fmt, {v1, v2, v3});
}

std::string format_impl_4(
    const std::string& fmt,
    emscripten::val v1,
    emscripten::val v2,
    emscripten::val v3,
    emscripten::val v4
)
{
    return vformat_impl(fmt, {v1, v2, v3, v4});
}

std::string format_impl_5(
    const std::string& fmt,
    emscripten::val v1,
    emscripten::val v2,
    emscripten::val v3,
    emscripten::val v4,
    emscripten::val v5
)
{
    return vformat_impl(fmt, {v1, v2, v3, v4, v5});
}

std::string format_impl_6(
    const std::string& fmt,
    emscripten::val v1,
    emscripten::val v2,
    emscripten::val v3,
    emscripten::val v4,
    emscripten::val v5,
    emscripten::val v6
)
{
    return vformat_impl(fmt, {v1, v2, v3, v4, v5, v6});
}

std::string format_impl_7(
    const std::string& fmt,
    emscripten::val v1,
    emscripten::val v2,
    emscripten::val v3,
    emscripten::val v4,
    emscripten::val v5,
    emscripten::val v6,
    emscripten::val v7
)
{
    return vformat_impl(fmt, {v1, v2, v3, v4, v5, v6, v7});
}

std::string format_impl_8(
    const std::string& fmt,
    emscripten::val v1,
    emscripten::val v2,
    emscripten::val v3,
    emscripten::val v4,
    emscripten::val v5,
    emscripten::val v6,
    emscripten::val v7,
    emscripten::val v8
)
{
    return vformat_impl(fmt, {v1, v2, v3, v4, v5, v6, v7, v8});
}

std::string format_impl_9(
    const std::string& fmt,
    emscripten::val v1,
    emscripten::val v2,
    emscripten::val v3,
    emscripten::val v4,
    emscripten::val v5,
    emscripten::val v6,
    emscripten::val v7,
    emscripten::val v8,
    emscripten::val v9
)
{
    return vformat_impl(fmt, {v1, v2, v3, v4, v5, v6, v7, v8, v9});
}

std::string format_impl_10(
    const std::string& fmt,
    emscripten::val v1,
    emscripten::val v2,
    emscripten::val v3,
    emscripten::val v4,
    emscripten::val v5,
    emscripten::val v6,
    emscripten::val v7,
    emscripten::val v8,
    emscripten::val v9,
    emscripten::val v10
)
{
    return vformat_impl(fmt, {v1, v2, v3, v4, v5, v6, v7, v8, v9, v10});
}
} // namespace em_papilio

EMSCRIPTEN_BINDINGS(papilio)
{
    using namespace papilio;
    namespace em = emscripten;

    em::constant("VERSION_MAJOR", PAPILIO_VERSION_MAJOR);
    em::constant("VERSION_MINOR", PAPILIO_VERSION_MINOR);
    em::constant("VERSION_PATCH", PAPILIO_VERSION_PATCH);

    em::class_<em_papilio::context>("context")
        .constructor<>()
        .function("push", &em_papilio::context::push)
        .function("format", &em_papilio::context::format);

#define EM_PAPILIO_EXPORT_FORMAT_IMPL(arg_count) \
    em::function("format", &em_papilio::format_impl_##arg_count);

    EM_PAPILIO_EXPORT_FORMAT_IMPL(0);
    EM_PAPILIO_EXPORT_FORMAT_IMPL(1);
    EM_PAPILIO_EXPORT_FORMAT_IMPL(2);
    EM_PAPILIO_EXPORT_FORMAT_IMPL(3);
    EM_PAPILIO_EXPORT_FORMAT_IMPL(4);
    EM_PAPILIO_EXPORT_FORMAT_IMPL(5);
    EM_PAPILIO_EXPORT_FORMAT_IMPL(6);
    EM_PAPILIO_EXPORT_FORMAT_IMPL(7);
    EM_PAPILIO_EXPORT_FORMAT_IMPL(8);
    EM_PAPILIO_EXPORT_FORMAT_IMPL(9);
    EM_PAPILIO_EXPORT_FORMAT_IMPL(10);

    em::constant("MAX_FORMAT_ARGS", 10);
}
