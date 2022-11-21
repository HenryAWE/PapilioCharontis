#include <gtest/gtest.h>
#include <papilio/detail/container.hpp>
#include <span>


TEST(TestDetailContainer, SmallVector)
{
    using papilio::detail::small_vector;

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
TEST(TestDetailContainer, FixedVector)
{
    using papilio::detail::fixed_vector;

    {
        fixed_vector<int, 2> fv;
        EXPECT_EQ(fv.size(), 0);
        EXPECT_TRUE(fv.empty());

        EXPECT_THROW(fv.at(0), std::out_of_range);
        EXPECT_THROW(std::as_const(fv).at(0), std::out_of_range);

        fv.push_back(0);
        EXPECT_EQ(fv.size(), 1);
        EXPECT_FALSE(fv.empty());
        EXPECT_EQ(fv[0], 0);
        EXPECT_EQ(std::as_const(fv)[0], 0);
        EXPECT_EQ(fv.at(0), 0);
        EXPECT_EQ(std::as_const(fv).at(0), 0);

        EXPECT_EQ(fv.front(), 0);
        EXPECT_EQ(std::as_const(fv).front(), 0);
        EXPECT_EQ(fv.back(), 0);
        EXPECT_EQ(std::as_const(fv).back(), 0);

        fv.push_back(1);
        EXPECT_EQ(fv.size(), 2);
        EXPECT_FALSE(fv.empty());
        EXPECT_EQ(fv[1], 1);
        EXPECT_EQ(std::as_const(fv)[1], 1);
        EXPECT_EQ(fv.at(1), 1);
        EXPECT_EQ(std::as_const(fv).at(1), 1);

        EXPECT_EQ(fv.front(), 0);
        EXPECT_EQ(std::as_const(fv).front(), 0);
        EXPECT_EQ(fv.back(), 1);
        EXPECT_EQ(std::as_const(fv).back(), 1);

        EXPECT_THROW(fv.push_back(2), std::out_of_range);
    }

    {
        fixed_vector<std::string, 2> fv;
        EXPECT_EQ(fv.size(), 0);
        EXPECT_TRUE(fv.empty());
        EXPECT_EQ(fv.capacity(), 2);

        EXPECT_THROW(fv.at(0), std::out_of_range);
        EXPECT_THROW(std::as_const(fv).at(0), std::out_of_range);

        fv.push_back("first");
        EXPECT_EQ(fv.size(), 1);
        EXPECT_FALSE(fv.empty());
        EXPECT_EQ(fv[0], "first");
        EXPECT_EQ(std::as_const(fv)[0], "first");
        EXPECT_EQ(fv.at(0), "first");
        EXPECT_EQ(std::as_const(fv).at(0), "first");

        EXPECT_EQ(fv.front(), "first");
        EXPECT_EQ(std::as_const(fv).front(), "first");
        EXPECT_EQ(fv.back(), "first");
        EXPECT_EQ(std::as_const(fv).back(), "first");

        fv.push_back("second");
        EXPECT_EQ(fv.size(), 2);
        EXPECT_FALSE(fv.empty());
        EXPECT_EQ(fv[1], "second");
        EXPECT_EQ(std::as_const(fv)[1], "second");
        EXPECT_EQ(fv.at(1), "second");
        EXPECT_EQ(std::as_const(fv).at(1), "second");

        EXPECT_EQ(fv.front(), "first");
        EXPECT_EQ(std::as_const(fv).front(), "first");
        EXPECT_EQ(fv.back(), "second");
        EXPECT_EQ(std::as_const(fv).back(), "second");

        EXPECT_THROW(fv.push_back("third"), std::out_of_range);
    }

    {
        fixed_vector<std::string, 4> fv;

        EXPECT_EQ(fv.insert(fv.end(), "world"), fv.begin());
        EXPECT_EQ(fv.back(), "world");
        EXPECT_EQ(fv.size(), 1);

        EXPECT_EQ(fv.insert(fv.begin(), "hello"), fv.begin());
        EXPECT_EQ(fv.front(), "hello");
        EXPECT_EQ(fv.size(), 2);

        EXPECT_EQ(fv.insert(fv.begin() + 1, "test"), fv.begin() + 1);
        EXPECT_EQ(fv.at(1), "test");
        EXPECT_EQ(fv.size(), 3);

        EXPECT_EQ(fv.insert(fv.begin(), "first"), fv.begin());
        EXPECT_EQ(fv.at(0), "first");
        EXPECT_EQ(fv.size(), 4);

        EXPECT_THROW(fv.insert(fv.begin(), "overflow"), std::out_of_range);
        EXPECT_THROW(fv.insert(fv.end(), "overflow"), std::out_of_range);
    }
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
