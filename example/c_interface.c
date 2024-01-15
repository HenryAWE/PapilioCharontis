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

#define print_result(ctx, fmt, ...) \
    (papilio_format(ctx, fmt, __VA_ARGS__), write_result(ctx))

int main(void)
{
    papilio_context* ctx = papilio_create_context();

#ifdef PAPILIO_HAS_VA_OPT
    print_result(ctx, "PAPILIO_HAS_VA_OPT: {}", PAPILIO_HAS_VA_OPT);
    print_result(ctx, "{{plain text}}");
#else
    print_result(ctx, "PAPILIO_HAS_VA_OPT: {}", 0);

    papilio_clear_context(ctx);
    papilio_vformat(ctx, "{{plain text}}");
    write_result(ctx);
#endif

    print_result(ctx, "C_PAPILIO_MAX_FORMAT_ARGS: {}", C_PAPILIO_MAX_FORMAT_ARGS);
    print_result(ctx, "Multiple outputs: {{{}, {}, {}}}", 1, 2.2f, "string");
    print_result(ctx, "{}", 1);
    print_result(ctx, "{} {}", 1, 2);
    print_result(ctx, "{} {} {}", 1, 2, 3);
    print_result(ctx, "{} {} {} {}", 1, 2, 3, 4);
    print_result(ctx, "{} {} {} {} {}", 1, 2, 3, 4, 5);
    print_result(ctx, "{} {} {} {} {} {}", 1, 2, 3, 4, 5, 6);
    print_result(ctx, "{} {} {} {} {} {} {}", 1, 2, 3, 4, 5, 6, 7);
    print_result(ctx, "{} {} {} {} {} {} {} {}", 1, 2, 3, 4, 5, 6, 7, 8);
    print_result(ctx, "{} {} {} {} {} {} {} {} {}", 1, 2, 3, 4, 5, 6, 7, 8, 9);
    print_result(ctx, "{} {} {} {} {} {} {} {} {} {}", 1, 2, 3, 4, 5, 6, 7, 8, 9, 10);

    print_result(ctx, "There {${0}!=1:'are':'is'} {0} big apple{${0}>1:'s'}", 1);

    print_result(ctx, "There {${0}!=1:'are':'is'} {0} big apple{${0}>1:'s'}", 2);

    print_result(
        ctx,
        "String: \"{0}\", size = {0.size}, length = {0.length}\n"
        "First character = '{0[0]}' (U+{0[0]:04X})\n"
        "last character  = '{0[-1]}' (U+{0[-1]:04X})",
        "hello world"
    );

    papilio_destroy_context(ctx);
    ctx = NULL;

    return 0;
}
