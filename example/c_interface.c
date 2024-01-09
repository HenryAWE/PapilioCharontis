#include <stdio.h>
#include <c_papilio.h>

void write_result(const papilio_context* ctx)
{
    const size_t sz = papilio_get_str_size(ctx);
    const char* str = papilio_get_str(ctx);

    for(size_t i = 0; i < sz; ++i)
        putchar(str[i]);
    putchar('\n');
}

int main(void)
{
    papilio_context* ctx = papilio_create_context();

    papilio_format(ctx, "There {${0}!=1:'are':'is'} {0} big apple{${0}>1:'s'}", 1);
    write_result(ctx);

    papilio_format(ctx, "There {${0}!=1:'are':'is'} {0} big apple{${0}>1:'s'}", 2);
    write_result(ctx);

    papilio_format(
        ctx,
        "String: \"{0}\", size = {0.size}, length = {0.length}\n"
        "First character = '{0[0]}' (U+{0[0]:04X})\n"
        "last character  = '{0[-1]}' (U+{0[-1]:04X})",
        "hello world"
    );
    write_result(ctx);

    papilio_destroy_context(ctx);
    ctx = NULL;

    return 0;
}
