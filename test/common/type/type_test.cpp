#include <gtest/gtest.h>
#include <papilio/papilio.hpp>


using namespace papilio;

// Static tests
static_assert(common::detailed::is_char_v<char>);
static_assert(common::detailed::is_char_v<wchar_t>);
static_assert(common::detailed::is_char_v<char16_t>);
static_assert(common::detailed::is_char_v<char32_t>);
static_assert(common::detailed::is_char_v<char8_t>);
static_assert(!common::detailed::is_char_v<signed char>);
static_assert(!common::detailed::is_char_v<unsigned char>);
static_assert(common::detailed::is_string_v<std::string>);
static_assert(common::detailed::is_string_v<std::wstring>);
static_assert(common::detailed::is_string_v<std::u16string>);
static_assert(common::detailed::is_string_v<std::u32string>);
static_assert(common::detailed::is_string_v<std::u8string>);

static_assert(common::get_data_type<void>() == common::data_type::none_type);
static_assert(common::get_data_type<signed char>() == common::data_type::int_type);
static_assert(common::get_data_type<unsigned char>() == common::data_type::uint_type);
static_assert(common::get_data_type<short>() == common::data_type::int_type);
static_assert(common::get_data_type<unsigned short>() == common::data_type::uint_type);
static_assert(common::get_data_type<int>() == common::data_type::int_type);
static_assert(common::get_data_type<unsigned int>() == common::data_type::uint_type);
static_assert(common::get_data_type<long>() == common::data_type::long_type);
static_assert(common::get_data_type<unsigned long>() == common::data_type::ulong_type);
static_assert(common::get_data_type<long long>() == common::data_type::long_long_type);
static_assert(common::get_data_type<unsigned long long>() == common::data_type::ulong_long_type);
static_assert(common::get_data_type<float>() == common::data_type::float_type);
static_assert(common::get_data_type<double>() == common::data_type::double_type);
static_assert(common::get_data_type<long double>() == common::data_type::long_double_type);
static_assert(common::get_data_type<const char*>() == common::data_type::cstring_type);
static_assert(common::get_data_type<std::string>() == common::data_type::string_type);
static_assert(common::get_data_type<std::nullptr_t>() == common::data_type::pointer_type);
static_assert(common::get_data_type<void*>() == common::data_type::pointer_type);
class custom_class {};
static_assert(common::get_data_type<custom_class>() == common::data_type::custom_type);

TEST(TestType, Typeinfo)
{
    using namespace common;
    EXPECT_EQ(common_data_type(data_type::int_type, data_type::float_type), data_type::float_type);
    static_assert(to_unsigned(data_type::int_type) == data_type::uint_type);
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
