#include <gtest/gtest.h>
#include <cstring>
#include <papilio/memory.hpp>
#include <papilio/utility.hpp>
#include <papilio_test/setup.hpp>

static_assert(std::is_trivial_v<papilio::static_storage<4>>);
static_assert(std::is_standard_layout_v<papilio::static_storage<4>>);

static_assert(std::is_empty_v<papilio::static_storage<0>>);

static_assert(papilio::pointer_like<papilio::optional_unique_ptr<int>>);
static_assert(papilio::pointer_like<papilio::optional_unique_ptr<int[]>>);

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
} // namespace test_memory

TEST(OptionalUniquePtr, Ownership)
{
    using namespace papilio;
    using namespace test_memory;

    {
        using traits = std::pointer_traits<optional_unique_ptr<int>>;

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
}

TEST(OptionalUniquePtr, SwapMixedOwnership)
{
    using namespace papilio;

    {
        int stack_val = 7;
        optional_unique_ptr<int> owning = make_optional_unique<int>(1);
        optional_unique_ptr<int> borrowed(&stack_val, false);

        ASSERT_TRUE(owning.has_ownership());
        ASSERT_FALSE(borrowed.has_ownership());

        owning.swap(borrowed);

        // The pointers and the ownership flags must be swapped together.
        EXPECT_EQ(*owning, 7);
        EXPECT_FALSE(owning.has_ownership());
        EXPECT_EQ(*borrowed, 1);
        EXPECT_TRUE(borrowed.has_ownership());
        // Destruction must delete the heap object once and must not touch the stack object.
    }

    {
        // Swapping two owning pointers keeps both sides owning.
        optional_unique_ptr<int> a = make_optional_unique<int>(1);
        optional_unique_ptr<int> b = make_optional_unique<int>(2);
        a.swap(b);
        EXPECT_EQ(*a, 2);
        EXPECT_TRUE(a.has_ownership());
        EXPECT_EQ(*b, 1);
        EXPECT_TRUE(b.has_ownership());
    }

    {
        // Swapping two borrowed pointers keeps both sides borrowed.
        int x = 1;
        int y = 2;
        optional_unique_ptr<int> a(&x, false);
        optional_unique_ptr<int> b(&y, false);
        a.swap(b);
        EXPECT_EQ(*a, 2);
        EXPECT_FALSE(a.has_ownership());
        EXPECT_EQ(*b, 1);
        EXPECT_FALSE(b.has_ownership());
    }
}

TEST(OptionalUniquePtr, CopyWithDifferentDeleter)
{
    using namespace papilio;

    static int deleted = 0;
    deleted = 0;

    struct counting_deleter
    {
        void operator()(int* p) const noexcept
        {
            ++deleted;
            delete p;
        }
    };

    struct convertible_deleter
    {
        convertible_deleter(const counting_deleter&) {}

        void operator()(int*) const noexcept {}
    };

    {
        auto up = std::unique_ptr<int, counting_deleter>(
            new int(42),
            counting_deleter{}
        );
        optional_unique_ptr<int, counting_deleter> src(std::move(up));
        ASSERT_TRUE(src.has_ownership());

        // Converting to a different deleter must produce a non-owning copy
        optional_unique_ptr<int, convertible_deleter> copy(src);
        EXPECT_EQ(*copy, 42);
        EXPECT_FALSE(copy.has_ownership());

        copy.reset();
        EXPECT_EQ(deleted, 0); // The copy did not delete

        src.reset();
        EXPECT_EQ(deleted, 1); // The source deleted exactly once
    }
}

TEST(OptionalUniquePtr, Compatibility)
{
    using namespace papilio;
    using namespace test_memory;

    {
        using ptr_t = optional_unique_ptr<void*, c_deleter>;
        static_assert(std::same_as<ptr_t::pointer, c_deleter::pointer>);
        static_assert(std::same_as<ptr_t::pointer, void*>);
    }

    {
        optional_unique_ptr<void, c_deleter> p;
        EXPECT_FALSE(p.has_ownership());
        EXPECT_EQ(p, nullptr);
        EXPECT_EQ(nullptr, p);

        p.reset(std::malloc(4), true);
        std::memset(p.get(), 0, 4);
        ASSERT_TRUE(p.has_ownership());

        unsigned char buf[4] = {0, 0, 0, 0};
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
        optional_unique_ptr opt_int = std::make_unique<int>(42);

        static_assert(std::same_as<decltype(opt_int), optional_unique_ptr<int, std::default_delete<int>>>);

        ASSERT_TRUE(opt_int.has_ownership());
        EXPECT_EQ(*opt_int, 42);
    }

    {
        optional_unique_ptr opt_int = make_optional_unique<int>(42);

        static_assert(std::same_as<decltype(opt_int), optional_unique_ptr<int, std::default_delete<int>>>);

        ASSERT_TRUE(opt_int.has_ownership());
        EXPECT_EQ(*opt_int, 42);
    }

    {
        optional_unique_ptr<int[]> opt_int_arr(new int[4]{0, 1, 2, 3}, true);
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
        int arr[4] = {0, 1, 2, 3};
        optional_unique_ptr<int[]> opt_int_arr(arr, false);
        ASSERT_FALSE(opt_int_arr.has_ownership());

        for(std::size_t i = 0; i < 4; ++i)
            EXPECT_EQ(opt_int_arr[i], i);

        auto observer_arr_ptr = opt_int_arr;
        ASSERT_FALSE(observer_arr_ptr.has_ownership());
        EXPECT_EQ(observer_arr_ptr, opt_int_arr);
        EXPECT_EQ(opt_int_arr, observer_arr_ptr);
    }

    {
        optional_unique_ptr opt_int_arr = make_optional_unique<int[]>(4);
        for(std::size_t i = 0; i < 4; ++i)
            EXPECT_EQ(opt_int_arr[i], 0);
    }
}
