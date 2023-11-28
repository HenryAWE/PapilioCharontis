#include <gtest/gtest.h>
#include <algorithm>
#include <ranges>
#include <map>
#include <papilio/papilio.hpp>


TEST(TestScript, Lexer)
{
    using namespace papilio;
    using namespace script;

    static_assert(is_lexeme_v<lexeme::argument>);
    static_assert(is_lexeme_v<lexeme::identifier>);
    static_assert(is_lexeme_v<lexeme::constant>);
    static_assert(is_lexeme_v<lexeme::keyword>);
    static_assert(is_lexeme_v<lexeme::operator_>);

    {
        lexer l;
        l.parse(R"(if $0: 'one\'s')");

        auto lexemes = l.lexemes();
        
        EXPECT_EQ(lexemes[0].type(), lexeme_type::keyword);
        EXPECT_EQ(lexemes[0].as<lexeme::keyword>().get(), keyword_type::if_);

        EXPECT_EQ(lexemes[1].type(), lexeme_type::argument);
        EXPECT_EQ(lexemes[1].as<lexeme::argument>().get_index(), 0);

        EXPECT_EQ(lexemes[2].type(), lexeme_type::operator_);
        EXPECT_EQ(lexemes[2].as<lexeme::operator_>().get(), operator_type::colon);

        EXPECT_EQ(lexemes[3].type(), lexeme_type::constant);
        EXPECT_EQ(lexemes[3].as<lexeme::constant>().get_string(), "one's");
    }

    {
        lexer l;
        l.parse("if !$0: 'false'");

        auto lexemes = l.lexemes();
        EXPECT_EQ(lexemes.size(), 5);

        EXPECT_EQ(lexemes[1].type(), lexeme_type::operator_);
        EXPECT_EQ(lexemes[1].as<lexeme::operator_>().get(), operator_type::not_);

        EXPECT_EQ(lexemes[2].type(), lexeme_type::argument);
    }

    {
        lexer l;
        l.parse(R"($0[-1])");

        auto lexemes = l.lexemes();

        EXPECT_EQ(lexemes[0].type(), lexeme_type::argument);
        EXPECT_EQ(lexemes[0].as<lexeme::argument>().get_index(), 0);

        EXPECT_EQ(lexemes[1].type(), lexeme_type::operator_);
        EXPECT_EQ(lexemes[1].as<lexeme::operator_>().get(), operator_type::bracket_l);

        EXPECT_EQ(lexemes[2].type(), lexeme_type::constant);
        EXPECT_EQ(lexemes[2].as<lexeme::constant>().get_int(), -1);

        EXPECT_EQ(lexemes[3].type(), lexeme_type::operator_);
        EXPECT_EQ(lexemes[3].as<lexeme::operator_>().get(), operator_type::bracket_r);
    }

    {
        lexer l;
        l.parse(R"($0[-2:-1])");

        auto lexemes = l.lexemes();

        EXPECT_EQ(lexemes[0].type(), lexeme_type::argument);
        EXPECT_EQ(lexemes[0].as<lexeme::argument>().get_index(), 0);

        EXPECT_EQ(lexemes[1].type(), lexeme_type::operator_);
        EXPECT_EQ(lexemes[1].as<lexeme::operator_>().get(), operator_type::bracket_l);

        EXPECT_EQ(lexemes[2].type(), lexeme_type::constant);
        EXPECT_EQ(lexemes[2].as<lexeme::constant>().get_int(), -2);

        EXPECT_EQ(lexemes[3].type(), lexeme_type::operator_);
        EXPECT_EQ(lexemes[3].as<lexeme::operator_>().get(), operator_type::colon);

        EXPECT_EQ(lexemes[4].type(), lexeme_type::constant);
        EXPECT_EQ(lexemes[4].as<lexeme::constant>().get_int(), -1);

        EXPECT_EQ(lexemes[5].type(), lexeme_type::operator_);
        EXPECT_EQ(lexemes[5].as<lexeme::operator_>().get(), operator_type::bracket_r);
    }

    {
        lexer l;
        l.parse(R"($0[:-1])");

        auto lexemes = l.lexemes();
        EXPECT_EQ(lexemes.size(), 5);

        EXPECT_EQ(lexemes[0].type(), lexeme_type::argument);
        EXPECT_EQ(lexemes[0].as<lexeme::argument>().get_index(), 0);

        EXPECT_EQ(lexemes[1].type(), lexeme_type::operator_);
        EXPECT_EQ(lexemes[1].as<lexeme::operator_>().get(), operator_type::bracket_l);

        EXPECT_EQ(lexemes[2].type(), lexeme_type::operator_);
        EXPECT_EQ(lexemes[2].as<lexeme::operator_>().get(), operator_type::colon);

        EXPECT_EQ(lexemes[3].type(), lexeme_type::constant);
        EXPECT_EQ(lexemes[3].as<lexeme::constant>().get_int(), -1);

        EXPECT_EQ(lexemes[4].type(), lexeme_type::operator_);
        EXPECT_EQ(lexemes[4].as<lexeme::operator_>().get(), operator_type::bracket_r);
    }

    {
        lexer l;
        l.parse(R"($0[10][:3])");

        auto lexemes = l.lexemes();
        EXPECT_EQ(lexemes.size(), 8);

        EXPECT_EQ(lexemes[0].type(), lexeme_type::argument);
        EXPECT_EQ(lexemes[0].as<lexeme::argument>().get_index(), 0);

        EXPECT_EQ(lexemes[1].type(), lexeme_type::operator_);
        EXPECT_EQ(lexemes[1].as<lexeme::operator_>().get(), operator_type::bracket_l);

        EXPECT_EQ(lexemes[2].type(), lexeme_type::constant);
        EXPECT_EQ(lexemes[2].as<lexeme::constant>().get_int(), 10);

        EXPECT_EQ(lexemes[3].type(), lexeme_type::operator_);
        EXPECT_EQ(lexemes[3].as<lexeme::operator_>().get(), operator_type::bracket_r);

        EXPECT_EQ(lexemes[4].type(), lexeme_type::operator_);
        EXPECT_EQ(lexemes[4].as<lexeme::operator_>().get(), operator_type::bracket_l);

        EXPECT_EQ(lexemes[5].type(), lexeme_type::operator_);
        EXPECT_EQ(lexemes[5].as<lexeme::operator_>().get(), operator_type::colon);

        EXPECT_EQ(lexemes[6].type(), lexeme_type::constant);
        EXPECT_EQ(lexemes[6].as<lexeme::constant>().get_int(), 3);

        EXPECT_EQ(lexemes[7].type(), lexeme_type::operator_);
        EXPECT_EQ(lexemes[7].as<lexeme::operator_>().get(), operator_type::bracket_r);
    }

    // non-ASCII characters
    {
        lexer l;
        l.parse((const char*)u8R"('非ASCII字符串')");

        auto lexemes = l.lexemes();
        EXPECT_EQ(lexemes[0].type(), lexeme_type::constant);
        EXPECT_EQ(
            lexemes[0].as<lexeme::constant>().get_string(),
            (const char*)u8"非ASCII字符串"
        );
    }

    {
        lexer l;
        l.parse("$0.length");

        auto lexemes = l.lexemes();
        EXPECT_EQ(lexemes.size(), 3);

        EXPECT_EQ(lexemes[1].type(), lexeme_type::operator_);
        EXPECT_EQ(lexemes[1].as<lexeme::operator_>().get(), operator_type::dot);

        EXPECT_EQ(lexemes[2].type(), lexeme_type::identifier);
        EXPECT_EQ(lexemes[2].as<lexeme::identifier>().get(), "length");
    }

    {
        lexer l;
        l.parse(R"(if $name: {name} else: '(empty)')");
        auto lexemes = l.lexemes();

        EXPECT_EQ(lexemes[0].type(), lexeme_type::keyword);
        EXPECT_EQ(lexemes[0].as<lexeme::keyword>().get(), keyword_type::if_);

        EXPECT_EQ(lexemes[1].type(), lexeme_type::argument);
        EXPECT_EQ(lexemes[1].as<lexeme::argument>().get_string(), "name");

        EXPECT_EQ(lexemes[2].type(), lexeme_type::operator_);
        EXPECT_EQ(lexemes[2].as<lexeme::operator_>().get(), operator_type::colon);

        EXPECT_EQ(lexemes[3].type(), lexeme_type::field);
        EXPECT_EQ(lexemes[3].as<lexeme::field>().get(), "name");

        EXPECT_EQ(lexemes[4].type(), lexeme_type::keyword);
        EXPECT_EQ(lexemes[4].as<lexeme::keyword>().get(), keyword_type::else_);

        EXPECT_EQ(lexemes[5].type(), lexeme_type::operator_);
        EXPECT_EQ(lexemes[5].as<lexeme::operator_>().get(), operator_type::colon);

        EXPECT_EQ(lexemes[6].type(), lexeme_type::constant);
        EXPECT_EQ(lexemes[6].as<lexeme::constant>().get_string(), "(empty)");
    }

    {
        lexer l;
        l.parse(R"(if $0 == 0: 'zero')");

        auto lexemes = l.lexemes();

        EXPECT_EQ(lexemes[0].type(), lexeme_type::keyword);
        EXPECT_EQ(lexemes[0].as<lexeme::keyword>().get(), keyword_type::if_);

        EXPECT_EQ(lexemes[1].type(), lexeme_type::argument);
        EXPECT_EQ(lexemes[1].as<lexeme::argument>().get_index(), 0);

        EXPECT_EQ(lexemes[2].type(), lexeme_type::operator_);
        EXPECT_EQ(lexemes[2].as<lexeme::operator_>().get(), operator_type::equal);

        EXPECT_EQ(lexemes[3].type(), lexeme_type::constant);
        EXPECT_EQ(lexemes[3].as<lexeme::constant>().get_int(), 0);

        EXPECT_EQ(lexemes[4].type(), lexeme_type::operator_);
        EXPECT_EQ(lexemes[4].as<lexeme::operator_>().get(), operator_type::colon);
    }

    {
        lexer l;

        std::string src = "[]";
        // skip '['
        auto result = l.parse(std::string_view(src).substr(1), lexer_mode::script_block);
        EXPECT_EQ(result.parsed_char, 0);

        auto lexemes = l.lexemes();
        EXPECT_EQ(lexemes.size(), 0);
    }

    {
        lexer l;

        std::string src = "[if $0: 'test']";
        // skip '['
        auto result = l.parse(std::string_view(src).substr(1), lexer_mode::script_block);
        EXPECT_EQ(result.parsed_char, src.size() - 2);

        auto lexemes = l.lexemes();
        EXPECT_EQ(lexemes.size(), 4);
    }

    {
        lexer l;

        std::string src = "{0.length:}";
        // skip '{'
        auto result = l.parse(std::string_view(src).substr(1), lexer_mode::replacement_field);
        EXPECT_EQ(result.parsed_char, src.size() - 3);
        EXPECT_EQ(src[result.parsed_char + 1], ':');

        auto lexemes = l.lexemes();
        EXPECT_EQ(lexemes.size(), 3);

        EXPECT_EQ(lexemes[0].type(), lexeme_type::argument);
        EXPECT_EQ(lexemes[0].as<lexeme::argument>().get_index(), 0);
    }

    {
        lexer l;

        std::string src = "{0.length}";
        // skip '{'
        auto result = l.parse(std::string_view(src).substr(1), lexer_mode::replacement_field);
        EXPECT_EQ(result.parsed_char, src.size() - 2);
        EXPECT_EQ(src[result.parsed_char + 1], '}');

        auto lexemes = l.lexemes();
        EXPECT_EQ(lexemes.size(), 3);

        EXPECT_EQ(lexemes[0].type(), lexeme_type::argument);
        EXPECT_EQ(lexemes[0].as<lexeme::argument>().get_index(), 0);
    }

    {
        lexer l;

        std::string src = "{name.length:}";
        // skip '{'
        auto result = l.parse(std::string_view(src).substr(1), lexer_mode::replacement_field);
        EXPECT_EQ(result.parsed_char, src.size() - 3);
        EXPECT_EQ(src[result.parsed_char + 1], ':');

        auto lexemes = l.lexemes();
        EXPECT_EQ(lexemes.size(), 3);

        EXPECT_EQ(lexemes[0].type(), lexeme_type::argument);
        EXPECT_EQ(lexemes[0].as<lexeme::argument>().get_string(), "name");
    }

    {
        lexer l;

        std::string src = "{.length:}";
        // skip '{'
        auto result = l.parse(
            std::string_view(src).substr(1),
            lexer_mode::replacement_field,
            0
        );
        EXPECT_EQ(result.parsed_char, src.size() - 3);
        EXPECT_EQ(src[result.parsed_char + 1], ':');

        auto lexemes = l.lexemes();
        EXPECT_EQ(lexemes.size(), 3); // includes the inserted argument

        EXPECT_EQ(lexemes[0].type(), lexeme_type::argument);
        EXPECT_EQ(lexemes[0].as<lexeme::argument>().get_index(), 0);
    }

    {
        lexer l;

        std::string src = "{[:]:}";
        // skip '{'
        auto result = l.parse(
            std::string_view(src).substr(1),
            lexer_mode::replacement_field,
            0
        );
        EXPECT_EQ(result.parsed_char, src.size() - 3);
        EXPECT_EQ(src[result.parsed_char + 1], ':');

        auto lexemes = l.lexemes();
        EXPECT_EQ(lexemes.size(), 4); // includes the inserted argument

        EXPECT_EQ(lexemes[0].type(), lexeme_type::argument);
        EXPECT_EQ(lexemes[0].as<lexeme::argument>().get_index(), 0);
    }
}
TEST(TestScript, Executor)
{
    using namespace papilio;
    using namespace script;

    // a placeholder
    auto empty_arg_store = make_format_args();

    {
        executor::context ctx(empty_arg_store);
        ctx.push(1);
        EXPECT_EQ(ctx.top().get<executor::int_type>(), 1);
    }
    {
        executor::context ctx(empty_arg_store);
        ctx.push(2.0L);
        EXPECT_DOUBLE_EQ(ctx.top().get<executor::float_type>(), 2.0L);
    }
    {
        executor::context ctx(empty_arg_store);
        ctx.push("string");
        EXPECT_EQ(ctx.top().get<utf::string_container>(), "string");
    }

    {
        executor::context ctx(empty_arg_store);

        executor ex(std::in_place_type<executor::constant<executor::int_type>>, 2);
        ex(ctx);

        auto i = ctx.copy_and_pop().get<executor::int_type>();
        EXPECT_EQ(i, 2);
    }

    {
        using namespace std::literals;

        int a1 = 1;
        float a2 = 2.0f;
        std::string a3 = "test";
        mutable_format_args store(a1, a2, "string"_a = a3);
        executor::context ctx(store);

        executor ex1(std::in_place_type<executor::argument>, 0);
        ex1(ctx);

        EXPECT_EQ(ctx.copy_and_pop().get<executor::int_type>(), 1);

        executor ex2(std::in_place_type<executor::argument>, 1);
        ex2(ctx);

        EXPECT_DOUBLE_EQ(ctx.copy_and_pop().get<executor::float_type>(), 2.0);

        executor ex3(std::in_place_type<executor::argument>, "string"s);
        ex3(ctx);

        EXPECT_EQ(ctx.copy_and_pop().get<utf::string_container>(), "test");

        executor ex4(
            std::in_place_type<executor::argument>,
            "string"s,
            format_arg_access::member_storage{ attribute_name("length") }
        );
        ex4(ctx);

        EXPECT_EQ(ctx.copy_and_pop().get<executor::int_type>(), std::string("test").length());
    }

    {
        executor::context ctx(empty_arg_store);

        executor ex(
            std::in_place_type<executor::comparator<std::less<>>>,
            std::make_unique<executor::constant<executor::int_type>>(1),
            std::make_unique<executor::constant<executor::int_type>>(2)
        );
        ex(ctx);

        EXPECT_TRUE(ctx.copy_and_pop().as<bool>());
    }
}
TEST(TestScript, Interpreter)
{
    using namespace papilio;
    using namespace script;

    // constant value
    {
        interpreter intp;
        std::string result = intp.run("'hello'");
        EXPECT_EQ(result, "hello");
    }

    // named argument
    {
        interpreter intp;
        std::string result = intp.run("$string", "string"_a = "hello");
        EXPECT_EQ(result, "hello");
    }

    // if
    {
        interpreter intp;

        std::string src = "if $0: 'hello'";
        std::string result = intp.run(src, true);
        EXPECT_EQ(result, "hello");

        result = intp.run(src, false);
        EXPECT_EQ(result, "");
    }

    // if-else
    {
        interpreter intp;

        std::string src = "if $0: 'a' else: 'b'";
        std::string result = intp.run(src, true);
        EXPECT_EQ(result, "a");

        result = intp.run(src, false);
        EXPECT_EQ(result, "b");
    }

    // if-elif-else
    {
        interpreter intp;

        std::string src = "if $0: 'a' elif $1: 'b' else: 'c'";
        std::string result = intp.run(src, true, false);
        EXPECT_EQ(result, "a");

        result = intp.run(src, false, true);
        EXPECT_EQ(result, "b");

        result = intp.run(src, false, false);
        EXPECT_EQ(result, "c");
    }

    // logical not
    {
        interpreter intp;

        std::string src = "if !$0: 'false'";
        std::string result = intp.run(src, false);
        EXPECT_EQ(result, "false");
    }

    {
        interpreter intp;

        std::string src = "if $0.length == 2: 'two'";
        std::string result = intp.run(src, "12");
        EXPECT_EQ(result, "two");
        result = intp.run(src, "123");
        EXPECT_EQ(result, "");

        src = "if $0.size == 2: 'two byte'";
        result = intp.run(src, "12");
        EXPECT_EQ(result, "two byte");
    }

    {
        interpreter intp;

        std::string src = "if $0.length != $0.size: 'multibyte'";
        std::string result = intp.run(src, "ASCII string");
        EXPECT_EQ(result, "");
        // non-ASCII
        std::string non_ascii = (const char*)u8"非ASCII字符串";
        result = intp.run(src, non_ascii);
        EXPECT_EQ(result, "multibyte");
    }

    // indexing
    {
        interpreter intp;

        std::string result = intp.run("$0[0]", "hello");
        EXPECT_EQ(result, "h");
        result = intp.run("$0[4]", "hello");
        EXPECT_EQ(result, "o");

        std::string str = "argument";
        result = intp.run("$0[0]", str);
        EXPECT_EQ(result, "a");
        result = intp.run("$0[-1]", str);
        EXPECT_EQ(result, "t");
    }

    // indexing for non-ASCII characters
    {
        using namespace std::literals;
        interpreter intp;

        std::string result = intp.run("$0[0]", (const char*)u8"这是一个测试字符串");
        EXPECT_EQ(result, (const char*)u8"这");

        std::string str = (const char*)u8"测试参数";
        result = intp.run("$0[0]", str);
        EXPECT_EQ(result, (const char*)u8"测");
        result = intp.run("$0[1]", str);
        EXPECT_EQ(result, (const char*)u8"试");
        result = intp.run("$0[2]", str);
        EXPECT_EQ(result, (const char*)u8"参");
        result = intp.run("$0[3]", str);
        EXPECT_EQ(result, (const char*)u8"数");
        result = intp.run("$0[-1]", str);
        EXPECT_EQ(result, (const char*)u8"数");
        result = intp.run("$0[-2]", str);
        EXPECT_EQ(result, (const char*)u8"参");
    }

    // slicing
    {
        interpreter intp;
        
        std::string str = "hello world!";
        std::string result = intp.run("$0[0:5]", str);
        result = intp.run("$0[:]", str);
        EXPECT_EQ(result, "hello world!");
        result = intp.run("$0[:5]", str);
        EXPECT_EQ(result, "hello");
        result = intp.run("$0[6:]", str);
        EXPECT_EQ(result, "world!");
        result = intp.run("$0[6:-1]", str);
        EXPECT_EQ(result, "world");
    }

    // comparing
    {
        interpreter intp;

        std::string src = "if $0 == 0: 'zero'";
        std::string result = intp.run(src, 0);
        EXPECT_EQ(result, "zero");
        result = intp.run(src, 1);
        EXPECT_EQ(result, "");
    }

    {
        interpreter intp;

        std::string src = "if $0 == $0: 'always true'";
        std::string result = intp.run(src, 0);
        EXPECT_EQ(result, "always true");
        result = intp.run(src, "string");
        EXPECT_EQ(result, "always true");
    }

    {
        interpreter intp;

        std::string src = "if $0[-1] == 's': 'plural' else: 'single'";
        std::string result = intp.run(src, "students");
        EXPECT_EQ(result, "plural");
        result = intp.run(src, "student");
        EXPECT_EQ(result, "single");
    }

    {
        interpreter intp;
        auto acc = intp.access("0[0]");

        mutable_format_args store("testing");
        EXPECT_EQ(
            get<utf::codepoint>(acc.second.access(store[acc.first])),
            U't'
        );
    }

    {
        interpreter intp;
        auto acc = intp.access("string[0]");

        mutable_format_args store("string"_a = "testing");
        EXPECT_EQ(
            get<utf::codepoint>(acc.second.access(store[acc.first])),
            U't'
        );
    }

    {
        std::string src = "[if $0 != 1: 's']";
        lexer l;
        l.parse(src.substr(1), lexer_mode::script_block);
        
        interpreter intp;
        auto ex = intp.compile(l.lexemes());

        mutable_format_args store(0);
        executor::context ctx(std::move(store));

        ex(ctx);
        EXPECT_EQ(ctx.get_result(), "s");
    }
}
using map_type = std::map<int, std::string>;
namespace papilio
{
    template <>
    struct accessor<map_type>
    {
        using has_index = void;

        static format_arg get(const map_type& m, indexing_value::index_type i)
        {
            auto it = m.find(i);
            if(it == m.end())
                return format_arg(utf::string_container());
            return utf::string_container(it->second);
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
TEST(TestScript, CustomType)
{
    using namespace papilio;
    using namespace script;

    {
        map_type m;
        m[0] = "zero";
        format_arg_access acc({ 0 });

        format_arg fmt_arg = m;
        auto member = acc.access(fmt_arg);
        EXPECT_EQ(get<utf::string_container>(member), "zero");
    }

    {
        map_type m;
        m[1] = "one";

        interpreter intp;
        std::string result = intp.run("$0[1]", m);
        EXPECT_EQ(result, "one");
        result = intp.run("$0[1][0]", m);
        EXPECT_EQ(result, "o");

        m.clear();
        m[10] = "tenth";
        result = intp.run("$0[10][:3]", m);
        EXPECT_EQ(result, "ten");

        result = intp.run("if $0[10][:3]: 'test'", m);
        EXPECT_EQ(result, "test");

        result = intp.run("if $0[10][:3] == 'ten': 'begin with \"ten\"'", m);
        EXPECT_EQ(result, "begin with \"ten\"");
    }
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
