#include "PyConnext.hpp"
#include <rti/core/Cookie.hpp>

using namespace rti::core;

template<>
void pyrti::init_class_defs(py::class_<Cookie>& cls) {
    cls
        .def(
            py::init<
                dds::core::vector<uint8_t>&
            >(),
            "bytes"
        )
        .def_property_readonly(
            "value",
            [](Cookie& c) {
                return std::vector<uint8_t>(c.value());
            },
            "Retrieve the bytes of the Cookie's contents."
        )
        .def(
            py::self == py::self,
            "Compare Cookies for equality"
        )
        .def(
            py::self != py::self,
            "Compare Cookies for inequality"
        )
        .doc() = 
            "Unique identifier for a written data sample in the form of a "
            "sequence of bytes.";
}

template<>
void pyrti::process_inits<Cookie>(py::module& m, pyrti::ClassInitList& l) {
    l.push_back(
        [m]() mutable {
            return pyrti::init_class<Cookie>(m, "Cookie");
        }
    );
}