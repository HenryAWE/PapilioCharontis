#include <gtest/gtest.h>
#include <papilio/memory.hpp>
#include <cstdlib>


TEST(TestMemory, Utilities)
{
    using namespace papilio;

    {
        static_assert(pointer_like<optional_ptr<int>>);
        static_assert(pointer_like<optional_ptr<int[]>>);
        static_assert(pointer_like<std::unique_ptr<int>>);
        static_assert(pointer_like<std::unique_ptr<int[]>>);
        static_assert(pointer_like<std::shared_ptr<int>>);
        static_assert(pointer_like<std::shared_ptr<int[]>>);
        static_assert(pointer_like<int*>);
        static_assert(pointer_like<int[]>);
        static_assert(!pointer_like<int>);
    }
}
namespace test_memory
{
    class c_deleter
    {
    public:
        using pointer = void*;

        void operator()(pointer p) const noexcept
        {
            std::free(p);
        }
    };
}
TEST(TestMemory, OptionalPtr)
{
    using namespace papilio;
    using namespace test_memory;
    using std::is_same_v;

    {
        using traits = std::pointer_traits<optional_ptr<int>>;

        int val = 42;
        auto ptr = traits::pointer_to(val);

        ASSERT_FALSE(ptr.has_ownership());
        EXPECT_EQ(*ptr, 42);
        EXPECT_EQ(*ptr, val);
        EXPECT_EQ(ptr.get(), &val);

        auto observer_ptr = ptr;
        EXPECT_EQ(observer_ptr, ptr);
        EXPECT_EQ(observer_ptr.get(), &val);

        ptr.reset();
        EXPECT_EQ(ptr, nullptr);
        EXPECT_EQ(nullptr, ptr);
        EXPECT_NE(observer_ptr, nullptr);
        EXPECT_NE(nullptr, observer_ptr);
    }

    {
        using ptr_t = optional_ptr<void*, c_deleter>;
        static_assert(is_same_v<ptr_t::pointer, c_deleter::pointer>);
        static_assert(std::is_same_v<ptr_t::pointer, void*>);
    }

    {
        optional_ptr<void, c_deleter> p;
        EXPECT_FALSE(p.has_ownership());
        EXPECT_EQ(p, nullptr);
        EXPECT_EQ(nullptr, p);

        p.reset(std::malloc(4));
        std::memset(p.get(), 0, 4);
        ASSERT_TRUE(p.has_ownership());

        unsigned char buf[4] = { 0, 0, 0, 0 };
        EXPECT_EQ(std::memcmp(p.get(), buf, 4), 0);

        auto observer_p = p;
        ASSERT_TRUE(p.has_ownership());
        ASSERT_FALSE(observer_p.has_ownership());
        EXPECT_EQ(observer_p, p);
        EXPECT_EQ(p, observer_p);

        observer_p.reset();
        ASSERT_TRUE(p.has_ownership());
        ASSERT_FALSE(observer_p.has_ownership());

        auto new_ptr = std::move(p);
        ASSERT_FALSE(p.has_ownership());
        ASSERT_TRUE(new_ptr.has_ownership());
    }

    {
        optional_ptr opt_int = std::make_unique<int>(42);

        static_assert(std::is_same_v<decltype(opt_int), optional_ptr<int, std::default_delete<int>>>);

        EXPECT_EQ(*opt_int, 42);
    }

    {
        optional_ptr<int[]> opt_int_arr(new int[4] { 0, 1, 2, 3 });
        ASSERT_TRUE(opt_int_arr.has_ownership());

        for(std::size_t i = 0; i < 4; ++i)
            EXPECT_EQ(opt_int_arr[i], i);

        auto observer_arr_ptr = opt_int_arr;
        ASSERT_FALSE(observer_arr_ptr.has_ownership());
        EXPECT_EQ(observer_arr_ptr, opt_int_arr);
        EXPECT_EQ(opt_int_arr, observer_arr_ptr);

        auto new_ptr = std::move(opt_int_arr);
        ASSERT_FALSE(opt_int_arr.has_ownership());
        ASSERT_TRUE(new_ptr.has_ownership());
    }

    {
        int arr[4] = { 0, 1, 2, 3 };
        optional_ptr<int[]> opt_int_arr(arr, false);
        ASSERT_FALSE(opt_int_arr.has_ownership());

        for(std::size_t i = 0; i < 4; ++i)
            EXPECT_EQ(opt_int_arr[i], i);

        auto observer_arr_ptr = opt_int_arr;
        ASSERT_FALSE(observer_arr_ptr.has_ownership());
        EXPECT_EQ(observer_arr_ptr, opt_int_arr);
        EXPECT_EQ(opt_int_arr, observer_arr_ptr);
    }
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
