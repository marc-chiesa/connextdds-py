#include "PyConnext.hpp"
#include <dds/core/xtypes/MemberType.hpp>

using namespace dds::core::xtypes;
using namespace rti::core::xtypes;

template<>
void pyrti::init_class_defs(py::class_<AbstractConstructedType<EnumMember>, DynamicType>& cls) {
    cls
        .def(
            "extensibility_kind", 
            &AbstractConstructedType<EnumMember>::extensibility_kind, 
            "Gets the extensibility kind."
        )
        .def_property_readonly(
            "member_count", 
            &AbstractConstructedType<EnumMember>::member_count, 
            "Gets the number of members."
        )
        .def(
            "member",
            (const EnumMember& (AbstractConstructedType<EnumMember>::*)(AbstractConstructedType<EnumMember>::MemberIndex) const) &AbstractConstructedType<EnumMember>::member,
            "Gets a member by its index."
        )
        .def(
            "member",
            (const EnumMember& (AbstractConstructedType<EnumMember>::*)(const std::string&) const) &AbstractConstructedType<EnumMember>::member,
            "Gets a member by its name."
        )
        .def(
            "find_member_by_name", 
            &AbstractConstructedType<EnumMember>::find_member_by_name, 
            "Obtains the member index from its name."
        )
        .def(
            "members", 
            &AbstractConstructedType<EnumMember>::members, 
            "Gets a copy of all the members"
        )
        .def(
            "cdr_serialized_sample_max_size", 
            &AbstractConstructedType<EnumMember>::cdr_serialized_sample_max_size, 
            "Gets the maximum serialized size of samples of this type."
        )
        .def_readonly_static(
            "INVALID_INDEX", 
            &AbstractConstructedType<EnumMember>::INVALID_INDEX, 
            "Indicates a member doesn't exist"
        );
}

template<>
void pyrti::process_inits<AbstractConstructedType<EnumMember>>(py::module& m, pyrti::ClassInitList& l) {
    l.push_back(
        [m]() mutable {
            return pyrti::init_class<AbstractConstructedType<EnumMember>, DynamicType>(m, "AbstractConstructedType<EnumMember>");
        }
    ); 
}