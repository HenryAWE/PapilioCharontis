module;
#include <papilio/script.hpp>


export module papilio.script;
export import <string>;
export import <string_view>;
export import <optional>;
export import papilio;

namespace papilio::script
{
    export class interpreter;
    export class variable;
}
