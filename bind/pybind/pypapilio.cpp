#include <pybind11/pybind11.h>
#include <papilio/papilio.hpp>
#include <papilio/util/all.hpp>


namespace papilio
{
    template <>
    class formatter<pybind11::handle>
    {
    public:
        void parse(format_spec_parse_context& spec)
        {
            m_spec = spec;
        }
        template <typename Context>
        void format(const pybind11::handle& h, Context& ctx)
        {
            auto result = h.attr("__format__").call(m_spec);

            format_context_traits traits(ctx);
            traits.append(result.cast<std::string>());
        }

    private:
        std::string_view m_spec;
    };

    template <>
    struct accessor<pybind11::handle>
    {
        static format_arg get_attr(pybind11::handle h, const attribute_name& attr)
        {
            using namespace std::literals;
            namespace py = pybind11;

            if(attr == "length"sv)
            {
                return format_arg(py::len(h));
            }
            else if(attr == "size"sv)
            {
                return format_arg(py::len(h));
            }

            throw invalid_attribute(attr);
        }
    };
}

namespace detail
{
    static thread_local std::vector<pybind11::handle> cached_handles;
    static thread_local std::vector<std::string> cached_names;
    static pybind11::str format_impl(std::string_view fmt, pybind11::args args, pybind11::kwargs kwargs)
    {
        namespace py = pybind11;

        cached_handles.clear();
        for(auto&& i : args)
            cached_handles.push_back(i);

        papilio::dynamic_format_arg_store store;
        for(const auto& i : cached_handles)
            store.emplace(i);

        cached_names.clear();
        for(auto&& [name, value] : kwargs)
        {
            cached_handles.push_back(value);
            cached_names.push_back(name.cast<std::string>());
            auto a = papilio::arg(
                cached_names.back().c_str(),
                cached_handles.back()
            );
            store.emplace(std::move(a));
        }

        return papilio::vformat(fmt, store);
    }
}

PYBIND11_MODULE(pypapilio, m)
{
    namespace py = pybind11;

    m.def(
        "format",
        detail::format_impl,
        py::arg("fmt")
    );
}
