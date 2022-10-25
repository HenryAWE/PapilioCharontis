#include <gtest/gtest.h>
#include <list>
#include <map>
#include <papilio/core.hpp>


TEST(TestCore, SmallVector)
{
    using papilio::small_vector;

    {
        small_vector<int, 8> sv;

        static_assert(sv.static_size() == 8);
        for(int i = 0; i < 8; ++i)
        {
            sv.emplace_back(i);
            EXPECT_EQ(sv[i], i);
            EXPECT_EQ(sv.at(i), i);
            EXPECT_EQ(std::as_const(sv)[i], i);
            EXPECT_EQ(std::as_const(sv).at(i), i);

            EXPECT_EQ(sv.front(), 0);
            EXPECT_EQ(std::as_const(sv).front(), 0);
            EXPECT_EQ(sv.back(), i);
            EXPECT_EQ(std::as_const(sv).back(), i);

            EXPECT_EQ(sv.size(), i + 1);
            EXPECT_EQ(sv.capacity(), sv.static_size());
            EXPECT_FALSE(sv.dynamic_allocated());
        }

        ASSERT_THROW(sv.at(8), std::out_of_range);
        ASSERT_THROW(std::as_const(sv).at(8), std::out_of_range);

        sv.emplace_back(8);
        EXPECT_EQ(sv.size(), 9);
        EXPECT_GT(sv.capacity(), sv.static_size());
        EXPECT_TRUE(sv.dynamic_allocated());
        EXPECT_EQ(sv.back(), 8);

        sv.assign({ 0, 1, 2, 3 });
        EXPECT_EQ(sv.size(), 4);
        EXPECT_TRUE(sv.dynamic_allocated());
        for(auto&& i : { 0, 1, 2, 3 })
        {
            EXPECT_EQ(sv[i], i);
        }

        sv.shrink_to_fit();
        EXPECT_FALSE(sv.dynamic_allocated());
    }

    {
        std::list<int> il{ 0, 1, 2, 3, 4, 5 };
        small_vector<int, 6> sv(il.begin(), il.end());
        EXPECT_FALSE(sv.dynamic_allocated());

        EXPECT_TRUE(std::equal(sv.begin(), sv.end(), il.begin(), il.end()));
        EXPECT_TRUE(std::equal(sv.cbegin(), sv.cend(), il.begin(), il.end()));
        EXPECT_TRUE(std::equal(sv.rbegin(), sv.rend(), il.rbegin(), il.rend()));
        EXPECT_TRUE(std::equal(sv.crbegin(), sv.crend(), il.rbegin(), il.rend()));
    }

    {
        small_vector<std::string, 4> sv{
            "one",
            "two",
            "three",
            "four"
        };
        EXPECT_FALSE(sv.dynamic_allocated());

        sv.assign({ "first", "second" });
        EXPECT_FALSE(sv.dynamic_allocated());
        sv.push_back("third");
        EXPECT_FALSE(sv.dynamic_allocated());
        sv.push_back("fourth");
        EXPECT_FALSE(sv.dynamic_allocated());
        sv.push_back("fifth");
        EXPECT_TRUE(sv.dynamic_allocated());
        EXPECT_EQ(sv.size(), 5);

        sv.pop_back();
        EXPECT_EQ(sv.size(), 4);
        EXPECT_TRUE(sv.dynamic_allocated());

        sv.shrink_to_fit();
        EXPECT_FALSE(sv.dynamic_allocated());
    }

    {
        small_vector<std::string, 2> sv_1{ "one", "two", "three" };
        EXPECT_TRUE(sv_1.dynamic_allocated());
        small_vector<std::string, 2> sv_2(std::move(sv_1));
        EXPECT_FALSE(sv_1.dynamic_allocated());
        EXPECT_TRUE(sv_1.empty());
        EXPECT_EQ(sv_1.capacity(), sv_1.static_size());
        EXPECT_TRUE(sv_2.dynamic_allocated());
        EXPECT_GE(sv_2.capacity(), 3);

        EXPECT_EQ(sv_2.at(0), "one");
        EXPECT_EQ(sv_2.at(1), "two");
        EXPECT_EQ(sv_2.at(2), "three");
        ASSERT_THROW(sv_2.at(3), std::out_of_range);

        small_vector<std::string, 2> sv_3{ "one" };
        EXPECT_FALSE(sv_3.dynamic_allocated());
        small_vector<std::string, 2> sv_4(std::move(sv_3));
        EXPECT_TRUE(sv_3.empty());
        EXPECT_FALSE(sv_4.dynamic_allocated());

        EXPECT_EQ(sv_4.at(0), "one");
        ASSERT_THROW(sv_4.at(1), std::out_of_range);

        small_vector<std::string, 2> sv_5;
        sv_5 = sv_2;
        EXPECT_EQ(sv_5.size(), 3);
        EXPECT_EQ(sv_5.back(), "three");
        EXPECT_TRUE(std::equal(sv_5.begin(), sv_5.end(), sv_2.begin(), sv_2.end()));

        sv_5 = std::move(sv_4);
        EXPECT_TRUE(sv_4.empty());
        EXPECT_EQ(sv_5.size(), 1);
        EXPECT_EQ(sv_5.at(0), "one");
    }
}
TEST(TestCore, Variable)
{
    using namespace papilio;
    using namespace script;

    static_assert(is_variable_type_v<variable::int_type>);
    static_assert(is_variable_type_v<variable::float_type>);
    static_assert(!is_variable_type_v<variable::string_type>);
    static_assert(is_variable_type_v<string_container>);

    {
        variable var = 10;
        EXPECT_TRUE(var.holds<variable::int_type>());
        EXPECT_EQ(var.get<variable::int_type>(), 10);
    }

    {
        variable var = "test";
        EXPECT_TRUE(var.holds<string_container>());
        EXPECT_EQ(var.get<string_container>(), "test");
        EXPECT_TRUE(var.as<bool>());
    }

    {
        variable var = std::string("test");
        EXPECT_TRUE(var.holds<string_container>());
        EXPECT_EQ(var.get<string_container>(), "test");
        EXPECT_TRUE(var.as<bool>());
    }

    {
        variable var1 = 2;
        variable var2 = 2.1f;
        EXPECT_LT(var1, var2);
    }

    {
        variable var1 = 1.0f;
        variable var2 = 1.0f;
        EXPECT_EQ(var1, var2);
    }

    {
        variable var1 = 1.0f;
        variable var2 = 1;
        EXPECT_EQ(var1, var2);
    }

    {
        variable var1 = "1";
        variable var2 = 1;
        EXPECT_NE(var1, var2);
    }
}
TEST(TestCore, Utilities)
{
    using namespace std::literals;

    {
        using namespace papilio;

        static_assert(string_like<char*>);
        static_assert(string_like<const char*>);
        static_assert(string_like<char[10]>);
        static_assert(string_like<const char[10]>);
        static_assert(string_like<std::string>);
        static_assert(string_like<std::string_view>);
    }

    {
        using namespace papilio;

        static_assert(!accessor_traits<int>::has_index());
        static_assert(!accessor_traits<float>::has_index());
        static_assert(accessor_traits<string_container>::has_index());
        static_assert(accessor_traits<string_container>::has_custom_index());
        static_assert(!accessor_traits<string_container>::has_key());
        static_assert(accessor_traits<string_container>::has_slice());
        static_assert(accessor_traits<string_container>::has_custom_slice());

        string_container test = "hello world";
        EXPECT_EQ(accessor_traits<string_container>::get(test, 0), U'h');
        EXPECT_EQ(accessor_traits<string_container>::get(test, slice(6, slice::npos)), "world");

        EXPECT_EQ(
            accessor_traits<string_container>::get_arg(test, -1).as_variable(),
            "d"
        );
        EXPECT_EQ(
            accessor_traits<string_container>::get_attr(test, "length").as_variable(),
            test.length()
        );
        EXPECT_EQ(
            accessor_traits<string_container>::get_attr(test, "size").as_variable(),
            test.size()
        );
    }

    {
        using namespace papilio;

        const std::string str_val = "hello world";
        const auto a_0 = arg("string", str_val);
        EXPECT_STREQ(a_0.name, "string");
        EXPECT_EQ(a_0.value, "hello world");

        const int int_val = 1;
        const auto a_1 = "integer"_a = int_val;
        EXPECT_STREQ(a_1.name, "integer");
        EXPECT_EQ(a_1.value, int_val);
    }
}
TEST(TestCore, AttributeName)
{
    using namespace std::literals;
    using namespace papilio;
    
    auto attr = attribute_name("name");

    EXPECT_EQ("name"sv, attr);
    EXPECT_EQ(attr, "name"sv);

    EXPECT_TRUE(attribute_name::validate("name"));
    EXPECT_TRUE(attribute_name::validate("_name"));
    EXPECT_TRUE(attribute_name::validate("NAME"));
    EXPECT_TRUE(attribute_name::validate("NAME_123"));

    EXPECT_FALSE(attribute_name::validate("123name"));
    EXPECT_FALSE(attribute_name::validate("NAME name"));
    EXPECT_FALSE(attribute_name::validate("$name"));
    EXPECT_FALSE(attribute_name::validate("!name"));
}
TEST(TestCore, FormatArg)
{
    using namespace std::literals;
    using namespace papilio;

    {
        format_arg arg('a');
        EXPECT_TRUE(arg.holds<utf8::codepoint>());
        EXPECT_EQ(get<utf8::codepoint>(arg), U'a');
    }

    {
        format_arg arg(1);
        EXPECT_TRUE(arg.holds<int>());
        EXPECT_EQ(get<int>(arg), 1);
    }

    {
        format_arg arg(1.0);
        EXPECT_TRUE(arg.holds<double>());
        EXPECT_DOUBLE_EQ(get<double>(arg), 1.0);
    }

    {
        papilio::format_arg fmt_arg("test");
        EXPECT_TRUE(fmt_arg.holds<string_container>());
        EXPECT_TRUE(get<string_container>(fmt_arg).is_borrowed());

        EXPECT_EQ(get<std::size_t>(fmt_arg.attribute("length")), std::string("test").length());
        EXPECT_EQ(get<utf8::codepoint>(fmt_arg.index(0)), U't');
        EXPECT_EQ(get<utf8::codepoint>(fmt_arg.index(1)), U'e');
        EXPECT_EQ(get<utf8::codepoint>(fmt_arg.index(2)), U's');
        EXPECT_EQ(get<utf8::codepoint>(fmt_arg.index(3)), U't');
        EXPECT_EQ(get<utf8::codepoint>(fmt_arg.index(4)), utf8::codepoint());
    }

    {
        using namespace std::literals;
        format_arg fmt_arg("test"s);
        EXPECT_TRUE(fmt_arg.holds<string_container>());
        EXPECT_FALSE(get<string_container>(fmt_arg).is_borrowed());
    }

    {
        using namespace std::literals;
        std::string s = "test"s;
        format_arg fmt_arg(s);
        EXPECT_TRUE(fmt_arg.holds<string_container>());
        EXPECT_TRUE(get<string_container>(fmt_arg).is_borrowed());
    }

    {
        papilio::format_arg fmt_arg((const char*)u8"测试");
        EXPECT_TRUE(fmt_arg.holds<string_container>());

        EXPECT_EQ(get<std::size_t>(fmt_arg.attribute("length")), 2);
        EXPECT_EQ(get<utf8::codepoint>(fmt_arg.index(0)), U'测');
        EXPECT_EQ(get<utf8::codepoint>(fmt_arg.index(1)), U'试');
        EXPECT_EQ(get<utf8::codepoint>(fmt_arg.index(2)), utf8::codepoint());
    }

    {
        papilio::format_arg fmt_arg("test");

        auto var = fmt_arg.as_variable();
        EXPECT_EQ(var.as<string_container>(), "test");
    }

    {
        papilio::format_arg fmt_arg("long sentence for testing slicing");

        EXPECT_EQ(get<string_container>(fmt_arg.index(slice(0, 4))), "long");
        EXPECT_EQ(get<string_container>(fmt_arg.index(slice(-7, slice::npos))), "slicing");
        EXPECT_EQ(get<string_container>(fmt_arg.index(slice(14, -16))), "for");
        EXPECT_EQ(get<string_container>(fmt_arg.index(slice(-slice::npos, -20))), "long sentence");

        EXPECT_EQ(get<std::string>(fmt_arg.index(slice(0, 4))), "long");
        EXPECT_EQ(get<std::string_view>(fmt_arg.index(slice(0, 4))), "long");
    }
}

using map_type = std::map<std::string, std::string, std::less<>>;
namespace papilio
{
    template <>
    struct accessor<map_type>
    {
        using has_key = void;

        static format_arg get(const map_type& m, std::string_view k)
        {
            auto it = m.find(k);
            if(it == m.end())
                return format_arg();
            return string_container(it->second);
        }

        static format_arg get_attr(const map_type& m, const attribute_name& attr)
        {
            using namespace std::literals;
            if(attr == "size"sv)
                return m.size();
            else
                throw invalid_attribute(attr);
        }
    };
}
TEST(TestCore, FormatArgHandle)
{
    using namespace papilio;

    {
        static_assert(!detail::use_handle<int>);
        static_assert(!detail::use_handle<char>);
        static_assert(detail::use_handle<map_type>);

        static_assert(accessor_traits<map_type>::has_key());

        map_type m;
        m["1"] = "one";
        m["2"] = "two";

        EXPECT_EQ(get<std::size_t>(accessor_traits<map_type>::get_attr(m, "size")), 2);
        EXPECT_EQ(get<string_container>(accessor_traits<map_type>::get_arg(m, "1")), "one");
        EXPECT_EQ(get<string_container>(accessor_traits<map_type>::get_arg(m, "2")), "two");
        EXPECT_TRUE(accessor_traits<map_type>::get_arg(m, "3").empty());

        format_arg::handle h = m;
        EXPECT_EQ(get<std::size_t>(h.attribute("size")), 2);
        EXPECT_EQ(get<string_container>(h.index("1")), "one");
        EXPECT_EQ(get<string_container>(h.index("2")), "two");

        format_arg arg = m;
        EXPECT_EQ(get<std::size_t>(arg.attribute("size")), 2);
        EXPECT_EQ(get<string_container>(arg.index("1")), "one");
        EXPECT_EQ(get<string_container>(arg.index("2")), "two");
    }
}
TEST(TestCore, DynamicFormatArgStore)
{
    using namespace papilio;

    {
        dynamic_format_arg_store store(1, "three"_a = 3, 2);

        EXPECT_EQ(store.size(), 2);
        EXPECT_EQ(store.named_size(), 1);

        EXPECT_EQ(get<int>(store[0]), 1);
        EXPECT_EQ(get<int>(store[1]), 2);
        EXPECT_EQ(get<int>(store["three"]), 3);

        store.clear();

        EXPECT_EQ(store.size(), 0);
        EXPECT_EQ(store.named_size(), 0);

        store.emplace('a', 'b', "c"_a = 'c', "d"_a = 'd');

        EXPECT_EQ(store.size(), 2);
        EXPECT_EQ(store.named_size(), 2);

        EXPECT_EQ(get<utf8::codepoint>(store[0]), U'a');
        EXPECT_EQ(get<utf8::codepoint>(store[1]), U'b');
        EXPECT_EQ(get<utf8::codepoint>(store["c"]), U'c');
        EXPECT_EQ(get<utf8::codepoint>(store["d"]), U'd');
    }

    {
        dynamic_format_arg_store store(1, 2.0f, "string", "named"_a = "named");

        EXPECT_EQ(get<std::size_t>(store[2].attribute("length")), 6);

        EXPECT_EQ(get<int>(store[indexing_value(0)]), 1);
        EXPECT_FLOAT_EQ(get<float>(store[indexing_value(1)]), 2.0f);
        EXPECT_EQ(get<string_container>(store[indexing_value("named")]), "named");
    }

    {
        dynamic_format_arg_store store(0, 1, 2, "test1"_a = "test 1", "test2"_a = "test 2");
        EXPECT_EQ(store.size(), 3);
        EXPECT_EQ(store.named_size(), 2);

        for(std::size_t i : { 0, 1, 2 })
            EXPECT_TRUE(store.check(i));
        EXPECT_FALSE(store.check(3));

        EXPECT_TRUE(store.check("test1"));
        EXPECT_TRUE(store.check("test2"));
        EXPECT_FALSE(store.check("test3"));
    }
}
TEST(TestCore, FormatContext)
{
    using namespace papilio;

    {
        std::string result;
        dynamic_format_arg_store store;
        basic_format_context ctx(
            std::back_inserter(result),
            store
        );
        format_context_traits traits(ctx);
        EXPECT_EQ(&traits.get_store(), &store);

        traits.append("1234");
        EXPECT_EQ(result, "1234");

        result.clear();
        traits.append('1', 4);
        EXPECT_EQ(result, "1111");

        result.clear();
        traits.append(U'ä', 2);
        EXPECT_EQ(result, (const char*)u8"ää");
    }

    {
        std::string result;
        dynamic_format_arg_store store;
        basic_format_context ctx(
            std::back_inserter(result),
            store
        );
        dynamic_format_context dyn_ctx(ctx);
        format_context_traits traits(dyn_ctx);
        EXPECT_EQ(&traits.get_store(), &store);

        traits.append("1234");
        EXPECT_EQ(result, "1234");

        result.clear();
        traits.append('1', 4);
        EXPECT_EQ(result, "1111");

        result.clear();
        traits.append(U'ä', 2);
        EXPECT_EQ(result, (const char*)u8"ää");
    }
}
TEST(TestCore, CommonFormatSpec)
{
    using namespace papilio;

    auto helper = []<typename... Args>(std::string_view fmt, Args&&... args)
    {
        common_format_spec spec;
        dynamic_format_arg_store store(std::forward<Args>(args)...);
        format_parse_context parse_ctx(fmt, store);
        parse_ctx.next_arg_id();
        format_spec_parse_context spec_ctx(parse_ctx, fmt.substr(2, fmt.size() - 3));

        spec.parse(spec_ctx);
        return spec;
    };

    {
        common_format_spec spec;
        spec = helper("{:}", 0);
        EXPECT_FALSE(spec.has_type_char());
        EXPECT_EQ(spec.type_char_or('d'), 'd');

        spec = helper("{: }", 0);
        EXPECT_FALSE(spec.has_type_char());
        EXPECT_EQ(spec.sign(), format_sign::space);

        spec = helper("{:d}", 0);
        EXPECT_EQ(spec.precision(), -1);
        EXPECT_EQ(spec.width(), 0);
        EXPECT_EQ(spec.type_char(), 'd');

        spec = helper("{:.10d}", 0);
        EXPECT_EQ(spec.precision(), 10);
        EXPECT_EQ(spec.type_char(), 'd');

        spec = helper("{:10d}", 0);
        EXPECT_EQ(spec.width(), 10);
        EXPECT_EQ(spec.type_char(), 'd');

        spec = helper("{:10.6d}", 0);
        EXPECT_EQ(spec.width(), 10);
        EXPECT_EQ(spec.precision(), 6);
        EXPECT_EQ(spec.type_char(), 'd');

        spec = helper("{: >}", "");
        EXPECT_FALSE(spec.has_type_char());
        EXPECT_EQ(spec.fill(), ' ');
        EXPECT_EQ(spec.align(), format_align::right);
    }
    {
        common_format_spec spec;

        spec = helper("{:.{}d}", 0, 10);
        EXPECT_EQ(spec.precision(), 10);
        EXPECT_EQ(spec.type_char(), 'd');

        spec = helper("{:{}d}", 0, 10);
        EXPECT_EQ(spec.width(), 10);
        EXPECT_EQ(spec.type_char(), 'd');

        spec = helper("{:{}.{}d}", 0, 10, 6);
        EXPECT_EQ(spec.width(), 10);
        EXPECT_EQ(spec.precision(), 6);
        EXPECT_EQ(spec.type_char(), 'd');
    }
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
