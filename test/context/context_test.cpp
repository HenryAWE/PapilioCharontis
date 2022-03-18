#include <gtest/gtest.h>
#include <papilio/papilio.hpp>


using namespace papilio;

TEST(TestContext, Argument)
{
    script::context::argument arg(2LL);
    EXPECT_TRUE(arg.compare(3) < 0);
    EXPECT_TRUE(arg.compare(1) > 0);
    EXPECT_TRUE(arg.compare(2) == 0);
    EXPECT_TRUE(arg.compare("2") == 0);
    EXPECT_EQ(arg.type(), typeid(decltype(2LL)));
    arg.assign(4LL);
    EXPECT_EQ(arg, 4);

    script::context::argument arg1(2.2f);
    EXPECT_LT(arg1, 4.4f);
    EXPECT_GT(arg, arg1);
    EXPECT_EQ(arg1.as<int>(), 2);
    arg1.assign(arg);
    EXPECT_EQ(arg, arg1);

    script::context::argument str_arg("test");
    EXPECT_EQ(str_arg.as<std::string>(), "test");
    auto str1 = std::string("str1");
    script::context::argument str_arg1(str1);
    EXPECT_EQ(str_arg1.as<std::string>(), "str1");

    script::wcontext::argument warg(2);
    EXPECT_TRUE(warg.compare(L"2") == 0);
    EXPECT_FALSE(warg.empty());
    warg.clear();
    EXPECT_TRUE(warg.empty());

    script::context::argument null_arg;
    EXPECT_FALSE(null_arg.as<bool>());
    EXPECT_TRUE(null_arg.as<std::string>().empty());
    EXPECT_EQ(null_arg.as<int>(), 0);
    EXPECT_TRUE(std::isnan(null_arg.as<float>()));
    EXPECT_TRUE(null_arg.empty());
}
TEST(TestContext, Value)
{
    using namespace script;

    context::value val(2);
    EXPECT_EQ(val.as<int>(), 2);
    val.assign("233");
    EXPECT_EQ(val.as<std::string>(), "233");
    context::argument arg(0);
    EXPECT_EQ(context::value(arg), arg.as<int>());
}
TEST(TestContext, ContextArgument)
{
    using namespace script;
    script::context ctx;
    ctx.push_arg(context::argument(0));
    ctx.push_arg(context::argument(1));
    ctx.set_named_arg("str", context::argument("named"));

    EXPECT_EQ(ctx.arg(0), 0);
    EXPECT_EQ(ctx.arg(1), 1);
    EXPECT_EQ(ctx.arg("str"), "named");
}
TEST(TestContext, Helpers)
{
    using namespace script;
    EXPECT_TRUE(helper::less()(1 <=> 2));
    EXPECT_FALSE(helper::less()(2 <=> 2));
    EXPECT_FALSE(helper::less()(2 <=> 1));

    EXPECT_TRUE(helper::less_equal()(1 <=> 2));
    EXPECT_TRUE(helper::less_equal()(2 <=> 2));
    EXPECT_FALSE(helper::less_equal()(2 <=> 1));

    EXPECT_TRUE(helper::greater()(2 <=> 1));
    EXPECT_FALSE(helper::greater()(2 <=> 2));
    EXPECT_FALSE(helper::greater()(1 <=> 2));

    EXPECT_TRUE(helper::greater_equal()(2 <=> 1));
    EXPECT_TRUE(helper::greater_equal()(2 <=> 2));
    EXPECT_FALSE(helper::greater_equal()(1 <=> 2));

    EXPECT_TRUE(helper::equal()(1 <=> 1));
    EXPECT_FALSE(helper::equal()(1 <=> 2));
    EXPECT_FALSE(helper::equal()(2 <=> 1));
    EXPECT_FALSE(helper::equal()(NAN <=> NAN));

    EXPECT_TRUE(helper::not_equal()(1 <=> 2));
    EXPECT_TRUE(helper::not_equal()(2 <=> 1));
    EXPECT_FALSE(helper::not_equal()(2 <=> 2));
    EXPECT_TRUE(helper::not_equal()(NAN <=> NAN));
}

using Context = script::context;
//Helper
template <typename Script, typename... Args>
std::unique_ptr<Context::script> make_script(Args&&... args)
{
    return std::unique_ptr<Context::script>(new Script(std::forward<Args>(args)...));
}

TEST(TestContext, Execution)
{
    using namespace script;

    Context::script_if if_;
    if_.condition = make_script<Context::script_argument<bool>>(0);
    if_.on_true = make_script<Context::script_literal>(12);
    if_.on_false = make_script<Context::script_literal>(23);

    Context ctx;
    ctx.push_arg(Context::argument(true));
    EXPECT_EQ(if_.invoke(ctx), 12);
    ctx.clear_arg();
    ctx.push_arg(Context::argument(false));
    EXPECT_EQ(if_.invoke(ctx), 23);
    ctx.clear_arg();

    auto comp = std::make_unique<Context::script_compare<helper::less>>();
    comp->left_operand = make_script<Context::script_argument_any>("int");
    comp->right_operand = make_script<Context::script_literal>(5);
    if_.condition.reset(comp.release());
    ctx.set_named_arg("int", Context::argument(2));
    EXPECT_EQ(if_.invoke(ctx), 12);
    ctx.clear_arg();
    ctx.set_named_arg("int", Context::argument(6));
    EXPECT_EQ(if_.invoke(ctx), 23);
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
