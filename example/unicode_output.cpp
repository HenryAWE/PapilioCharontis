#include <papilio/papilio.hpp>


int main()
{
    using namespace papilio;

    println("{:.^10s}", (const char*)u8"äëïöü");
    println("{:.^10s}", (const char*)u8"测试");
    println("{:.^10s}", (const char*)u8"テスト");

    return 0;
}
