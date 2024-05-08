#include <gtest/gtest.h>
#include <papilio/container.hpp>
#include <list>
#include <papilio_test/setup.hpp>

TEST(small_vector, emplace)
{
    using papilio::small_vector;

    {
        small_vector<int, 8> sv;

        {
            auto it = sv.emplace(sv.end(), 1);
            EXPECT_EQ(it, sv.end() - 1);
        }

        {
            auto it = sv.insert(sv.begin(), 2);
            EXPECT_EQ(it, sv.begin());
        }

        EXPECT_EQ(sv[0], 2);
        EXPECT_EQ(sv[1], 1);
        EXPECT_EQ(sv.size(), 2);
    }

    {
        small_vector<int, 4> sv{2, 4, 6, 8};

        sv.insert(sv.begin(), 0);
        EXPECT_EQ(sv.size(), 5);
        EXPECT_EQ(sv.front(), 0);

        for(int i = 0; i < 5; ++i)
            EXPECT_EQ(sv[i], i * 2);
    }

    {
        small_vector<std::string, 4> sv{"one", "two", "three"};

        {
            auto it = sv.insert(sv.begin(), "zero");
            EXPECT_EQ(it, sv.begin());
        }

        EXPECT_EQ(sv.size(), 4);
        EXPECT_EQ(sv.front(), "zero");

        {
            auto it = sv.insert(sv.end(), "four");
            EXPECT_EQ(it, sv.end() - 1);
        }

        EXPECT_EQ(sv.size(), 5);
        EXPECT_TRUE(sv.dynamic_allocated());
        EXPECT_EQ(sv.back(), "four");
    }
}

TEST(small_vector, emplace_back)
{
    using papilio::small_vector;

    small_vector<int, 8> sv;

    static_assert(sv.static_capacity() == 8);
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
        EXPECT_EQ(sv.capacity(), sv.static_capacity());
        EXPECT_FALSE(sv.dynamic_allocated());
    }

    ASSERT_THROW(sv.at(8), std::out_of_range);
    ASSERT_THROW(std::as_const(sv).at(8), std::out_of_range);

    sv.emplace_back(8);
    EXPECT_EQ(sv.size(), 9);
    EXPECT_GT(sv.capacity(), sv.static_capacity());
    EXPECT_TRUE(sv.dynamic_allocated());
    EXPECT_EQ(sv.back(), 8);

    sv.assign({0, 1, 2, 3});
    EXPECT_EQ(sv.size(), 4);
    EXPECT_TRUE(sv.dynamic_allocated());
    for(auto&& i : {0, 1, 2, 3})
    {
        EXPECT_EQ(sv[i], i);
    }

    sv.shrink_to_fit();
    EXPECT_FALSE(sv.dynamic_allocated());

    int arr[4] = {4, 5, 6, 7};
    sv.append_range(arr);

    for(int i = 0; i < 8; ++i)
        EXPECT_EQ(sv[i], i);
}

TEST(small_vector, iterator)
{
    using papilio::small_vector;

    std::list<int> il{0, 1, 2, 3, 4, 5};
    small_vector<int, 6> sv(il.begin(), il.end());
    EXPECT_FALSE(sv.dynamic_allocated());

    EXPECT_TRUE(std::equal(sv.begin(), sv.end(), il.begin(), il.end()));
    EXPECT_TRUE(std::equal(sv.cbegin(), sv.cend(), il.begin(), il.end()));
    EXPECT_TRUE(std::equal(sv.rbegin(), sv.rend(), il.rbegin(), il.rend()));
    EXPECT_TRUE(std::equal(sv.crbegin(), sv.crend(), il.rbegin(), il.rend()));
}

TEST(small_vector, dynamic_allocated)
{
    using papilio::small_vector;

    small_vector<std::string, 4> sv{
        "one",
        "two",
        "three",
        "four"
    };
    EXPECT_FALSE(sv.dynamic_allocated());

    sv.assign({"first", "second"});
    EXPECT_NE(sv.front(), "one");

    EXPECT_FALSE(sv.dynamic_allocated());
    sv.push_back("third");
    EXPECT_FALSE(sv.dynamic_allocated());
    sv.push_back("fourth");
    EXPECT_FALSE(sv.dynamic_allocated());
    sv.push_back("fifth");
    EXPECT_TRUE(sv.dynamic_allocated());
    EXPECT_EQ(sv.size(), 5);

    sv.reserve(16);
    EXPECT_GE(sv.capacity(), 16);
    EXPECT_EQ(sv.size(), 5);
    EXPECT_EQ(sv.back(), "fifth");

    sv.shrink_to_fit();
    EXPECT_EQ(sv.capacity(), sv.size());

    sv.pop_back();
    EXPECT_EQ(sv.size(), 4);
    EXPECT_TRUE(sv.dynamic_allocated());

    sv.shrink_to_fit();
    EXPECT_FALSE(sv.dynamic_allocated());
    EXPECT_EQ(sv.capacity(), sv.static_capacity());
}

TEST(small_vector, constructor)
{
    using papilio::small_vector;

    small_vector<std::string, 2> sv_1{"one", "two", "three"};
    EXPECT_TRUE(sv_1.dynamic_allocated());
    small_vector<std::string, 2> sv_2(std::move(sv_1));
    EXPECT_FALSE(sv_1.dynamic_allocated());
    EXPECT_TRUE(sv_1.empty());
    EXPECT_EQ(sv_1.capacity(), sv_1.static_capacity());
    EXPECT_TRUE(sv_2.dynamic_allocated());
    EXPECT_GE(sv_2.capacity(), 3);

    EXPECT_EQ(sv_2.at(0), "one");
    EXPECT_EQ(sv_2.at(1), "two");
    EXPECT_EQ(sv_2.at(2), "three");
    ASSERT_THROW(sv_2.at(3), std::out_of_range);

    small_vector<std::string, 2> sv_3{"one"};
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

TEST(small_vector, swap)
{
    using papilio::small_vector;

    small_vector<std::string, 2> sv_1{"A", "B", "C"};
    small_vector<std::string, 2> sv_2{"a", "b"};

    EXPECT_TRUE(sv_1.dynamic_allocated());
    EXPECT_EQ(sv_1.size(), 3);
    EXPECT_FALSE(sv_2.dynamic_allocated());
    EXPECT_EQ(sv_2.size(), 2);

    sv_1.swap(sv_2);

    EXPECT_FALSE(sv_1.dynamic_allocated());
    EXPECT_EQ(sv_1.size(), 2);
    EXPECT_TRUE(sv_2.dynamic_allocated());
    EXPECT_EQ(sv_2.size(), 3);

    EXPECT_EQ(sv_1[0], "a");
    EXPECT_EQ(sv_1[1], "b");

    EXPECT_EQ(sv_2[0], "A");
    EXPECT_EQ(sv_2[1], "B");
    EXPECT_EQ(sv_2[2], "C");

    sv_2.pop_back();
    sv_2.shrink_to_fit();
    EXPECT_EQ(sv_2.size(), 2);
    EXPECT_FALSE(sv_2.dynamic_allocated());

    EXPECT_EQ(sv_1.size(), sv_2.size());
    sv_1.swap(sv_2);
    EXPECT_EQ(sv_1.size(), sv_2.size());

    sv_1.pop_back();
    EXPECT_EQ(sv_1.at(0), "A");
    EXPECT_EQ(sv_1.size(), 1);
    EXPECT_FALSE(sv_1.dynamic_allocated());

    sv_1.swap(sv_2);
    EXPECT_EQ(sv_2.at(0), "A");
    EXPECT_EQ(sv_2.size(), 1);
    EXPECT_EQ(sv_1.at(0), "a");
    EXPECT_EQ(sv_1.at(1), "b");
    EXPECT_EQ(sv_1.size(), 2);
}
