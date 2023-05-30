#include <gtest/gtest.h>
#include <papilio/memory.hpp>


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
    class my_deleter
    {
    public:
        using pointer = unsigned int;

        void operator()(pointer) const noexcept {}
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
        ptr.reset();
        EXPECT_EQ(ptr, nullptr);
        EXPECT_EQ(nullptr, ptr);
    }

    {
        using ptr_t = optional_ptr<int, my_deleter>;
        static_assert(is_same_v<ptr_t::pointer, my_deleter::pointer>);

        ptr_t p;
        EXPECT_FALSE(p.has_ownership());
        EXPECT_EQ(p, nullptr);
        EXPECT_EQ(nullptr, p);

        p.reset(0x1, true);
        EXPECT_TRUE(p.has_ownership());
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
    }

    {
        int arr[4] = { 0, 1, 2, 3 };
        optional_ptr<int[]> opt_int_arr(arr, false);
        ASSERT_FALSE(opt_int_arr.has_ownership());

        for(std::size_t i = 0; i < 4; ++i)
            EXPECT_EQ(opt_int_arr[i], i);
    }
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
