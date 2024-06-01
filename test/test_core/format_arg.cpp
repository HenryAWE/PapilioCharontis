#include <gtest/gtest.h>
#include <papilio/core.hpp>
#include <papilio_test/setup.hpp>

TEST(format_arg, constructor)
{
    using namespace papilio;

    {
        format_arg fmt_arg;
        EXPECT_TRUE(fmt_arg.empty());
        EXPECT_FALSE(fmt_arg);
        EXPECT_FALSE(fmt_arg.is_formattable());
    }

    {
        format_arg fmt_arg('a');
        EXPECT_TRUE(fmt_arg.holds<utf::codepoint>());
        EXPECT_EQ(get<utf::codepoint>(fmt_arg), U'a');
        EXPECT_TRUE(fmt_arg.has_ownership());
    }

    {
        format_arg fmt_arg(1);
        EXPECT_TRUE(fmt_arg.holds<int>());
        EXPECT_EQ(get<int>(fmt_arg), 1);
        EXPECT_TRUE(fmt_arg.has_ownership());
    }

    {
        format_arg fmt_arg(1.0);
        EXPECT_TRUE(fmt_arg.holds<double>());
        EXPECT_DOUBLE_EQ(get<double>(fmt_arg), 1.0);
        EXPECT_TRUE(fmt_arg.has_ownership());
    }

    {
        format_arg fmt_arg(nullptr);
        EXPECT_TRUE(fmt_arg.holds<const void*>());
        EXPECT_EQ(get<const void*>(fmt_arg), nullptr);
        EXPECT_TRUE(fmt_arg.has_ownership());
    }

    {
        void* mem = std::malloc(4);

        format_arg fmt_arg(mem);
        EXPECT_TRUE(fmt_arg.holds<const void*>());
        EXPECT_EQ(get<const void*>(fmt_arg), mem);
        EXPECT_TRUE(fmt_arg.has_ownership());

        std::free(mem);
    }

    {
        void* mem = std::malloc(4);

        format_arg fmt_arg(const_cast<const void*>(mem));
        EXPECT_TRUE(fmt_arg.holds<const void*>());
        EXPECT_EQ(get<const void*>(fmt_arg), mem);
        EXPECT_TRUE(fmt_arg.has_ownership());

        std::free(mem);
    }

    {
        int arr[4] = {0, 1, 2, 3};
        format_arg fmt_arg(arr);
        EXPECT_TRUE(fmt_arg.holds<format_arg::handle>());
        EXPECT_EQ(get<std::span<const int>>(fmt_arg).data(), arr);
    }

    {
        std::array<int, 4> arr = {0, 1, 2, 3};
        format_arg fmt_arg(arr);
        EXPECT_TRUE(fmt_arg.holds<format_arg::handle>());
        EXPECT_EQ(get<std::span<const int>>(fmt_arg).data(), arr.data());
    }

    {
        using namespace std::literals;

        PAPILIO_NS format_arg fmt_arg("test");
        EXPECT_TRUE(fmt_arg.holds<utf::string_container>());
        EXPECT_FALSE(get<utf::string_container>(fmt_arg).has_ownership());
        EXPECT_FALSE(fmt_arg.has_ownership());
    }

    {
        using namespace std::literals;

        PAPILIO_NS format_arg fmt_arg("test"s);
        EXPECT_TRUE(fmt_arg.holds<utf::string_container>());
        EXPECT_TRUE(get<utf::string_container>(fmt_arg).has_ownership());
        EXPECT_TRUE(fmt_arg.has_ownership());
    }

    {
        using namespace std::literals;

        std::string s = "test"s;
        format_arg fmt_arg(s);
        EXPECT_TRUE(fmt_arg.holds<utf::string_container>());
        EXPECT_FALSE(get<utf::string_container>(fmt_arg).has_ownership());
        EXPECT_FALSE(fmt_arg.has_ownership());
    }

    {
        std::map<int, int> m;

        static_assert(!detail::use_soo_handle<decltype(m)>);

        format_arg fmt_arg(m);
        EXPECT_FALSE(fmt_arg.has_ownership());
    }

    {
        using map_type = std::map<int, int>;

        format_arg fmt_arg;

        {
            map_type m = {
                {0, 0}
            };
            fmt_arg = format_arg(independent, std::move(m));
        }

        EXPECT_TRUE(fmt_arg.has_ownership());

        const auto& m = get<map_type>(fmt_arg);

        EXPECT_THROW(((void)get<std::map<int, float>>(fmt_arg)), bad_handle_cast);

        EXPECT_EQ(m.size(), 1);
        EXPECT_EQ(m.at(0), 0);
    }

    {
        struct int_wrapper
        {
            int v;
        };

        static_assert(detail::use_soo_handle<int_wrapper>);

        format_arg fmt_arg(int_wrapper{});
        EXPECT_TRUE(fmt_arg.has_ownership());
    }
}

TEST(format_arg, swap)
{
    // Not using namespace papilio to test ADL
    using papilio::utf::codepoint;

    {
        papilio::format_arg arg1('a');
        papilio::format_arg arg2('b');

        std::swap(arg1, arg2);

        EXPECT_EQ(get<codepoint>(arg1), U'b');
        EXPECT_EQ(get<codepoint>(arg2), U'a');
    }

    {
        papilio::format_arg arg1('a');
        papilio::format_arg arg2('b');

        // ADL
        swap(arg1, arg2);

        EXPECT_EQ(get<codepoint>(arg1), U'b');
        EXPECT_EQ(get<codepoint>(arg2), U'a');
    }
}

TEST(format_arg, access)
{
    using namespace papilio;

    {
        format_arg fmt_arg("test");
        EXPECT_TRUE(fmt_arg.holds<utf::string_container>());
        EXPECT_FALSE(get<utf::string_container>(fmt_arg).has_ownership());

        EXPECT_EQ(get<std::size_t>(fmt_arg.attribute("length")), std::string("test").length());
        EXPECT_EQ(get<utf::codepoint>(fmt_arg.index(0)), U't');
        EXPECT_EQ(get<utf::codepoint>(fmt_arg.index(1)), U'e');
        EXPECT_EQ(get<utf::codepoint>(fmt_arg.index(2)), U's');
        EXPECT_EQ(get<utf::codepoint>(fmt_arg.index(3)), U't');
        EXPECT_EQ(get<utf::codepoint>(fmt_arg.index(4)), utf::codepoint());
    }

    {
        // "测试", test in Chinese
        format_arg fmt_arg("\u6d4b\u8bd5");
        EXPECT_TRUE(fmt_arg.holds<utf::string_container>());

        EXPECT_EQ(get<std::size_t>(fmt_arg.attribute("length")), 2);
        EXPECT_EQ(get<utf::codepoint>(fmt_arg.index(0)), U'\u6d4b');
        EXPECT_EQ(get<utf::codepoint>(fmt_arg.index(1)), U'\u8bd5');
        EXPECT_EQ(get<utf::codepoint>(fmt_arg.index(2)), utf::codepoint());
    }

    {
        format_arg fmt_arg("test");

        auto var = variable(fmt_arg.to_variant());
        EXPECT_EQ(var.as<utf::string_container>(), "test");
    }

    {
        format_arg fmt_arg("long sentence for testing slicing");

        EXPECT_EQ(get<utf::string_container>(fmt_arg.index(slice(0, 4))), "long");
        EXPECT_EQ(get<utf::string_container>(fmt_arg.index(slice(-7, slice::npos))), "slicing");
        EXPECT_EQ(get<utf::string_container>(fmt_arg.index(slice(14, -16))), "for");
        EXPECT_EQ(get<utf::string_container>(fmt_arg.index(slice(-slice::npos, -20))), "long sentence");

        EXPECT_EQ(get<std::string>(fmt_arg.index(slice(0, 4))), "long");
        EXPECT_EQ(get<std::string_view>(fmt_arg.index(slice(0, 4))), "long");
    }
}
