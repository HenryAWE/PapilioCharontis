#include <gtest/gtest.h>
#include <papilio/papilio.hpp>
#include <papilio/util/stl_container.hpp>


TEST(TestSTLContainer, Tuple)
{
    using namespace papilio;

    {
        std::tuple<> empty_tp;

        static_assert(accessor_traits<std::tuple<>>::has_custom_index());

        EXPECT_EQ(papilio::format("{.size}", empty_tp), "0");
    }

    {
        using tuple_type = std::tuple<int, float, std::string>;
        tuple_type tp(0, 1.0f, "test");

        static_assert(accessor_traits<tuple_type>::has_custom_index());
        EXPECT_EQ(accessor_traits<tuple_type>::get_arg(tp, 0).as_variable(), 0);
        EXPECT_EQ(accessor_traits<tuple_type>::get_arg(tp, 1).as_variable(), 1.0f);
        EXPECT_EQ(accessor_traits<tuple_type>::get_arg(tp, 2).as_variable(), "test");
        EXPECT_TRUE(accessor_traits<tuple_type>::get_arg(tp, 4).empty());

        EXPECT_EQ(papilio::format("{.size}", tp), "3");
        EXPECT_EQ(papilio::format("{[0]}", tp), "0");
        EXPECT_EQ(papilio::format("{[-1]}", tp), "test");
    }

    {
        using pair_type = std::pair<int, std::string>;
        pair_type p(1, "hello");

        static_assert(accessor_traits<pair_type>::has_custom_index());
        EXPECT_EQ(accessor_traits<pair_type>::get_arg(p, 0).as_variable(), 1);
        EXPECT_EQ(accessor_traits<pair_type>::get_arg(p, 1).as_variable(), "hello");

        EXPECT_EQ(papilio::format("{.size}", p), "2");
        EXPECT_EQ(papilio::format("{0.first} == {0[0]}", p), "1 == 1");
        EXPECT_EQ(papilio::format("{0.second} == {0[1]}", p), "hello == hello");
    }
}
TEST(TestSTLContainer, Map)
{
    using namespace papilio;

    {
        std::map<std::string, int, std::less<>> m{
            { "one", 1 },
            { "two", 2 },
            { "three", 3 }
        };

        static_assert(accessor_traits<decltype(m)>::has_custom_key());
        EXPECT_EQ(accessor_traits<decltype(m)>::get_arg(m, "one").as_variable(), 1);
        EXPECT_EQ(accessor_traits<decltype(m)>::get_arg(m, "two").as_variable(), 2);
        EXPECT_EQ(accessor_traits<decltype(m)>::get_arg(m, "three").as_variable(), 3);

        // avoid collision with std::format caused by ADL
        EXPECT_EQ(papilio::format("{.size}", m), "3");
        EXPECT_EQ(papilio::format("{['one']}", m), "1");
        EXPECT_EQ(papilio::format("{['two']}", m), "2");
        EXPECT_EQ(papilio::format("{['three']}", m), "3");
    }

    {
        std::map<int, std::string, std::less<>> m{
            { 1, "one" },
            { 2, "two" },
            { 3, "three" }
        };

        static_assert(accessor_traits<decltype(m)>::has_custom_index());
        EXPECT_EQ(accessor_traits<decltype(m)>::get_arg(m, 1).as_variable(), "one");
        EXPECT_EQ(accessor_traits<decltype(m)>::get_arg(m, 2).as_variable(), "two");
        EXPECT_EQ(accessor_traits<decltype(m)>::get_arg(m, 3).as_variable(), "three");

        // avoid collision with std::format caused by ADL
        EXPECT_EQ(papilio::format("{.size}", m), "3");
        EXPECT_EQ(papilio::format("{[1]}", m), "one");
        EXPECT_EQ(papilio::format("{[2]}", m), "two");
        EXPECT_EQ(papilio::format("{[3]}", m), "three");
    }
}
TEST(TestSTLContainer, Vector)
{
    using namespace papilio;

    std::vector<int> ints{ 0, 1, 2, 3 };

    static_assert(accessor_traits<decltype(ints)>::has_custom_index());
    EXPECT_EQ(
        accessor_traits<decltype(ints)>::get_attr(ints, "size").as_variable(),
        ints.size()
    );
    for(std::size_t i = 0; i < ints.size(); ++i)
    {
        EXPECT_EQ(
            accessor_traits<decltype(ints)>::get_arg(ints, i).as_variable(),
            ints[i]
        );
    }

    // avoid collision with std::format caused by ADL
    EXPECT_EQ(papilio::format("{.size}", ints), "4");
    EXPECT_EQ(papilio::format("{[0]}", ints), "0");
    EXPECT_EQ(papilio::format("{[-1]}", ints), "3");
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
