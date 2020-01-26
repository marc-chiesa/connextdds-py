#include "PyConnext.hpp"
#include "PyEntity.hpp"
#include "PyAnyDataWriter.hpp"
#include "PyAnyDataReader.hpp"
#include "PyDataReader.hpp"
#include "PyDomainParticipantListener.hpp"
#include <rti/rti.hpp>

PYBIND11_MAKE_OPAQUE(std::vector<dds::topic::TopicBuiltinTopicData>);

using namespace dds::domain;

namespace pyrti {
    template<typename ParticipantFwdIterator>
    uint32_t find_participants(ParticipantFwdIterator begin)
    {
        DDS_DomainParticipantSeq native_participants = DDS_SEQUENCE_INITIALIZER;
        // Ensure sequence destruction
        rti::core::detail::NativeSequenceAdapter<DDS_DomainParticipant> participants_adapter(
            native_participants);

        DDS_ReturnCode_t retcode = DDS_DomainParticipantFactory_get_participants(
            DDS_DomainParticipantFactory_get_instance(),
            &native_participants);
        rti::core::check_return_code(retcode, "get native participants");

        rti::core::detail::create_from_native_entity<
            pyrti::PyDomainParticipant, DDS_DomainParticipant, ParticipantFwdIterator>(
                begin, participants_adapter);

        return participants_adapter.size();
    }
}

template<>
void pyrti::init_class_defs(py::class_<pyrti::PyDomainParticipant, pyrti::PyIEntity>& cls) {
    cls
        .def(
            py::init<int32_t>(),
            py::arg("domain_id"),
            "Create a new DomainParticipant with default QoS."
        )
        .def(
            py::init(
                [](
                    int32_t id, 
                    const qos::DomainParticipantQos& q, 
                    pyrti::PyDomainParticipantListener* l,
                    const dds::core::status::StatusMask& m) {
                        return pyrti::PyDomainParticipant(id, q, l, m);
                }),
            py::arg("domain_id"),
            py::arg("qos"),
            py::arg("the_listener") = (pyrti::PyDomainParticipantListener*) nullptr,
            py::arg_v("mask", dds::core::status::StatusMask::all(), "StatusMask.all()"),
            py::keep_alive<1, 4>(),
            "Create a new DomainParticipant"
        )
        .def(
            py::init(
                [](pyrti::PyIEntity& e) {
                    auto entity = e.get_entity();
                    return dds::core::polymorphic_cast<pyrti::PyDomainParticipant>(entity);
                }
            )
        )
        .def_property_readonly(
            "listener",
            [](const pyrti::PyDomainParticipant& dp) {
                return dynamic_cast<pyrti::PyDomainParticipantListener*>(dp.listener());
            },
            "Get the listener."
        )
        .def(
            "bind_listener",
            [](pyrti::PyDomainParticipant& dp, pyrti::PyDomainParticipantListener* l, const dds::core::status::StatusMask& m) {
                dp.listener(l, m);
            },
            py::arg("listener"),
            py::arg("event_mask"),
            py::keep_alive<1, 2>(),
            "Bind the listener and event mask to the DomainParticipant."
        )
        .def_property(
            "qos",
            (qos::DomainParticipantQos (pyrti::PyDomainParticipant::*)() const) &pyrti::PyDomainParticipant::qos,
            (void (pyrti::PyDomainParticipant::*)(const qos::DomainParticipantQos&)) &pyrti::PyDomainParticipant::qos,
            "Get a copy of or set the domain participant's QoS."
        )
        .def(
            "__lshift__",
            [](pyrti::PyDomainParticipant& dp, const dds::domain::qos::DomainParticipantQos& q) {
                return pyrti::PyDomainParticipant(dp << q);
            },
            py::is_operator(),
            "Set the domain participant's QoS."
        )
        .def(
            "__rshift__",
            [](pyrti::PyDomainParticipant& dp, dds::domain::qos::DomainParticipantQos& q) {
                return (dp >> q);
            },
            py::is_operator(),
            "Get the domain participant's QoS."
        )
        .def_property_readonly(
            "domain_id",
            &pyrti::PyDomainParticipant::domain_id,
            "The unique domain identifier."
        )
        .def("assert_liveliness", 
            &pyrti::PyDomainParticipant::assert_liveliness,
            "Manually assert the liveliness of the DomainParticipant."
        )
        .def(
            "contains_entity", 
            &pyrti::PyDomainParticipant::contains_entity, 
            py::arg("handle"),
            "Check whether or not the given handle represents an Entity that was created from the DomainParticipant."
        )
        .def_property_readonly(
            "current_time",
            &pyrti::PyDomainParticipant::current_time,
            "Get the current time."
        )
        .def_property(
            "default_publisher_qos",
            (dds::pub::qos::PublisherQos (pyrti::PyDomainParticipant::*)() const) &pyrti::PyDomainParticipant::default_publisher_qos,
            [](pyrti::PyDomainParticipant& dp, const dds::pub::qos::PublisherQos& q) {
                return pyrti::PyDomainParticipant(dp.default_publisher_qos(q));
            },
            "Get a copy of or set the default PublisherQos."
        )
        .def_property(
            "default_subscriber_qos",
            (dds::sub::qos::SubscriberQos (pyrti::PyDomainParticipant::*)() const) &pyrti::PyDomainParticipant::default_subscriber_qos,
            [](pyrti::PyDomainParticipant& dp, const dds::sub::qos::SubscriberQos& q) {
                return PyDomainParticipant(dp.default_subscriber_qos(q));
            },
            "Get a copy of or set the default SubscriberQos."
        )
        .def_property(
            "default_topic_qos",
            (dds::topic::qos::TopicQos (pyrti::PyDomainParticipant::*)() const) &pyrti::PyDomainParticipant::default_topic_qos,
            [](pyrti::PyDomainParticipant& dp, const dds::topic::qos::TopicQos& q) {
                return pyrti::PyDomainParticipant(dp.default_topic_qos(q));
            },
            "Get a copy of or set the default TopicQos."
        )
        .def_property(
            "default_datawriter_qos",
            [](const pyrti::PyDomainParticipant& dp) {
                return dp->default_datawriter_qos();
            },
            [](pyrti::PyDomainParticipant& dp, const dds::pub::qos::DataWriterQos &qos) {
                return dp->default_datawriter_qos(qos);
            },
            "Get a copy of or set the default DataWriterQos."
        )
        .def_property(
            "default_datareader_qos",
            [](const pyrti::PyDomainParticipant& dp) {
                return dp->default_datareader_qos();
            },
            [](pyrti::PyDomainParticipant& dp, const dds::sub::qos::DataReaderQos &qos) {
                return dp->default_datareader_qos(qos);
            },
            "Get a copy of or set the default DataReaderQos."
        )
        .def(
            "register_contentfilter",
            [](pyrti::PyDomainParticipant& dp, const rti::topic::CustomFilter<rti::topic::ContentFilterBase>& cf, const std::string &fn) {
                return dp->register_contentfilter(cf, fn);
            },
            py::arg("expression"),
            py::arg("name"),
            "Register a content filter which can be used to create a ContentFiltertedTopic."
        )
        .def(
            "unregister_contentfilter",
            [](pyrti::PyDomainParticipant& dp, const std::string &fn) {
                return dp->unregister_contentfilter(fn);
            },
            py::arg("name"),
            "Unregister content filter previously registered to this DomainParticipant."
        )
        .def(
            "register_type",
            [](pyrti::PyDomainParticipant& dp, 
                const std::string& n, 
                const dds::core::xtypes::DynamicType& t, 
                const rti::core::xtypes::DynamicDataTypeSerializationProperty& sp) {
                    rti::domain::register_type(dp, n, t, sp);
                },
            py::arg("name"),
            py::arg("type"),
            py::arg_v("serialization_property", rti::core::xtypes::DynamicDataTypeSerializationProperty::DEFAULT, "DynamicDataTypeSerializationProperty.DEFAULT"),
            "Registers a DynamicType with specific serialization properties."
        )
        .def(
            "unregister_type",
            [](pyrti::PyDomainParticipant& dp, const std::string& name) {
                return dp->unregister_type(name);
            },
            py::arg("name"),
            "Unregister a type previously registered to this DomainParticipant."
        )
        .def(
            "is_type_registered",
            [](const pyrti::PyDomainParticipant& dp, const std::string& name) {
                return dp->is_type_registered(name);
            },
            py::arg("name"),
            "Check if a type has been registered to this DomainParticipant."
        )
        .def(
            "add_peer",
            [](pyrti::PyDomainParticipant& dp, const std::string& peer_desc_str) {
                dp->add_peer(peer_desc_str);
            },
            py::arg("peer"),
            "Attempt to contact an additional peer participant."
        )
        .def(
            "add_peers",
            [](pyrti::PyDomainParticipant& dp, const std::vector<std::string>& peer_list) {
                for (auto& p : peer_list) {
                    dp->add_peer(p);
                }
            },
            py::arg("peers"),
            "Add a sequence of peers to be contacted."
        )
        .def(
            "remove_peer",
            [](pyrti::PyDomainParticipant& dp, const std::string& peer_desc_str) {
                dp->remove_peer(peer_desc_str);
            },
            py::arg("peer"),
            "Remove a peer participant from this list that this DomainParticipant will attempt to communicate with."
        )
        .def(
            "remove_peers",
            [](pyrti::PyDomainParticipant& dp, const std::vector<std::string>& peer_list) {
                for (auto& p : peer_list) {
                    dp->remove_peer(p);
                }
            },
            py::arg("peers"),
            "Remove a sequence of peers from the contact list."
        )
        .def(
            "resume_endpoint_discovery",
            [](pyrti::PyDomainParticipant& dp, const dds::core::InstanceHandle& rph) {
                dp->resume_endpoint_discovery(rph);
            },
            py::arg("handle"),
            "Initiates endpoint discovery with the remote DomainParticipant identified by its InstanceHandle."
        )
        .def(
            "delete_durable_subscription",
            [](pyrti::PyDomainParticipant& dp, const rti::core::EndpointGroup& group) {
                dp->delete_durable_subscription(group);
            },
            py::arg("group"),
            "Deletes an existing Durable Subscription on all Persistence Services."
        )
        .def(
            "register_durable_subscription",
            [](pyrti::PyDomainParticipant& dp, const rti::core::EndpointGroup& group, const std::string& topic_name) {
                dp->register_durable_subscription(group, topic_name);
            },
            py::arg("group"),
            py::arg("topic_name"),
            "Registers a Durable Subscription on the specified Topic on all Persistence Services"
        )
        .def_property_readonly(
            "participant_protocol_status",
            [](pyrti::PyDomainParticipant& dp) {
                return dp->participant_protocol_status();
            },
            "Get the protocol status for this participant"
        )
        .def_property_static(
            "participant_factory_qos",
            (qos::DomainParticipantFactoryQos (*)()) &pyrti::PyDomainParticipant::participant_factory_qos,
            (void (*)(const dds::domain::qos::DomainParticipantFactoryQos& qos)) &pyrti::PyDomainParticipant::participant_factory_qos,
            "Get a copy of or set the DomainParticipantFactoryQos."
        )
        .def_static(
            "finalize_participant_factory",
            &pyrti::PyDomainParticipant::finalize_participant_factory,
            "Finalize the DomainParticipantFactory"
        )
        .def_property_static(
            "default_participant_qos",
            (qos::DomainParticipantQos (*)()) &pyrti::PyDomainParticipant::default_participant_qos,
            (void (*)(const qos::DomainParticipantQos& qos)) &pyrti::PyDomainParticipant::default_participant_qos,
            "Get a copy of or set the default DomainParticipantQos."
        )
        .def(
            "__enter__", 
            [](pyrti::PyDomainParticipant& dp) {
                return dp;
            },
            "Enter a context for this Domain Participant, to be cleaned up on exiting context"
        )
        .def(
            "__exit__",
            [](pyrti::PyDomainParticipant& dp, py::object, py::object, py::object) {
                dp.close();
            },
            "Exit the context for this Domain Participant, cleaning up resources."
        )
        .def_static(
            "find",
            [](const std::string& n) {
                pyrti::PyDomainParticipant(rti::domain::find_participant_by_name(n));
            },
            py::arg("name"),
            "Find a local DomainParticipant by its name."
        )
        .def_static(
            "find",
            []() {
                std::vector<pyrti::PyDomainParticipant> v;
                pyrti::find_participants(std::back_inserter(v));
                return v;
            },
            "Find all local DomainParticipants."
        )
        .def_static(
            "find",
            [](int32_t domain_id) {
                return pyrti::PyDomainParticipant(dds::domain::find(domain_id));
            },
            py::arg("domain_id"),
            "Find a local DomainParticipant with the given domain ID. If more "
            "than one DomainParticipant on the same domain exists in the "
            "application, it is not specified which will be returned."
        )
        .def(
            "find_publisher",
            [](pyrti::PyDomainParticipant& dp, const std::string& name) {
                return pyrti::PyPublisher(rti::pub::find_publisher(dp, name));
            },
            py::arg("name"),
            "Lookup a Publisher within the DomainParticipant by its entity name."
        )
        .def(
            "find_publishers",
            [](const pyrti::PyDomainParticipant& dp) {
                std::vector<pyrti::PyPublisher> v;
                rti::pub::find_publishers(dp, std::back_inserter(v));
                return v;
            },
            "Find all Publishers within the DomainParticipant."
        )
        .def_property_readonly(
            "implicit_publisher",
            [](pyrti::PyDomainParticipant& dp) {
                return pyrti::PyPublisher(rti::pub::implicit_publisher(dp));
            },
            "Get the implicit Publisher for the DomainParticipant."
        )
        .def_property_readonly(
            "builtin_subscriber",
            [](pyrti::PyDomainParticipant& dp) {
                return pyrti::PySubscriber(dds::sub::builtin_subscriber(dp));
            },
            "Get the built-in subscriber for the DomainParticipant."
        )
        .def(
            "find_subscriber",
            [](pyrti::PyDomainParticipant& dp, const std::string& name) {
                return pyrti::PySubscriber(rti::sub::find_subscriber(dp, name));
            },
            py::arg("name"),
            "Find a Subscriber in the DomainParticipant by its entity name."
        )
        .def(
            "find_subscribers",
            [](pyrti::PyDomainParticipant& dp) {
                std::vector<pyrti::PySubscriber> v;
                rti::sub::find_subscribers(dp, std::back_inserter(v));
                return v;
            },
            "Find all subscribers within the DomainParticipant."
        )
        .def_property_readonly(
            "implicit_subscriber",
            [](pyrti::PyDomainParticipant& dp) {
                return pyrti::PySubscriber(rti::sub::implicit_subscriber(dp));
            },
            "Get the implicit Subscriber for the DomainParticipant."
        )
        .def(
            "ignore_participant",
            (void (*)(const DomainParticipant&, const dds::core::InstanceHandle&)) &dds::domain::ignore,
            py::arg("handle"),
            "Ignore a DomainParticipant given it's handle."
        )
        .def(
            "ignore_participants",
            [](pyrti::PyDomainParticipant& dp, std::vector<dds::core::InstanceHandle>& v) {
                dds::domain::ignore(dp, v.begin(), v.end());
            }
        )
        .def(
            "ignore_topic",
            (void (*)(DomainParticipant&, const dds::core::InstanceHandle&)) &dds::topic::ignore,
            py::arg("handle"),
            "Ignore a Topic matching the provided handle."
        )
        .def(
            "ignore_topics",
            [](pyrti::PyDomainParticipant& dp, std::vector<dds::core::InstanceHandle>& v) {
                dds::topic::ignore(dp, v.begin(), v.end());
            },
            py::arg("handles"),
            "Ignore a list of Topics specified by their handles."
        )
        .def(
            "ignore_datawriter",
            (void (*)(DomainParticipant&, const dds::core::InstanceHandle&)) &dds::pub::ignore,
            py::arg("handle"),
            "Ignore a DataWriter matching the provided handle."
        )
        .def(
            "ignore_datawriters",
            [](pyrti::PyDomainParticipant& dp, std::vector<dds::core::InstanceHandle>& v) {
                dds::pub::ignore(dp, v.begin(), v.end());
            },
            py::arg("handles"),
            "Ignore a list of DataWriters specified by their handles."
        )
        .def(
            "ignore_datareader",
            (void (*)(DomainParticipant&, const dds::core::InstanceHandle&)) &dds::sub::ignore,
            py::arg("handle"),
            "Ignore a DataReader matching the provided handle."
        )
        .def(
            "ignore_datareaders",
            [](pyrti::PyDomainParticipant& dp, std::vector<dds::core::InstanceHandle>& v) {
                dds::sub::ignore(dp, v.begin(), v.end());
            },
            py::arg("handles"),
            "Ignore a list of DataReaders specified by their handles."
        )
        .def(
            "find_topics",
            [](pyrti::PyDomainParticipant& dp) {
                std::vector<pyrti::PyAnyTopic> v;
                rti::topic::find_topics(dp, std::back_inserter(v));
                return v;
            },
            "Find all Topics in the DomainParticipant."
        )
        .def(
            "discovered_topics",
            [](const pyrti::PyDomainParticipant& dp) {
                std::vector<dds::core::InstanceHandle> v;
                dds::topic::discover_any_topic(dp, std::back_inserter(v));
                return v;
            },
            "Get all Topic handles discovered by this DomainParticipant."
        )
        .def(
            "discovered_topic_data",
            (dds::topic::TopicBuiltinTopicData (*)(const DomainParticipant&, const dds::core::InstanceHandle&)) dds::topic::discover_topic_data,
            py::arg("handle"),
            "Get information about a discovered topic using its handle."
        )
        .def(
            "discovered_topic_data",
            [](const pyrti::PyDomainParticipant& dp, const pyrti::PyIEntity& topic) {
                return dds::topic::discover_topic_data(dp, topic.py_instance_handle());
            },
            py::arg("topic"),
            "Get information about a discovered topic."
        )
        .def(
            "discovered_topic_data",
            [](const pyrti::PyDomainParticipant& dp, const std::vector<dds::core::InstanceHandle>& handles) {
                std::vector<dds::topic::TopicBuiltinTopicData> v;
                for (auto& h : handles) {
                    v.push_back(dds::topic::discover_topic_data(dp, h));
                }
                return v;
            },
            py::arg("handles"),
            "Get information about a discovered topic."
        )
        .def(
            "discovered_topic_data",
            [](const pyrti::PyDomainParticipant& dp) {
                std::vector<dds::topic::TopicBuiltinTopicData> v;
                dds::topic::discover_topic_data(dp, std::back_inserter(v));
                return v;
            },
            "Get information about all discovered topics."
        )
        .def(
            "find_datawriter",
            [](pyrti::PyDomainParticipant& dp, const std::string name) {
                auto dw = rti::pub::find_datawriter_by_name<dds::pub::AnyDataWriter>(dp, name);
                return pyrti::PyAnyDataWriter(dw);
            },
            py::arg("name"),
            "Find a DataWriter by its name."
        )
        .def(
            "find_datareader",
            [](pyrti::PyDomainParticipant& dp, const std::string name) {
                auto dr = rti::sub::find_datareader_by_name<dds::sub::AnyDataReader>(dp, name);
                return pyrti::PyAnyDataReader(dr);
            },
            py::arg("name"),
            "Find a DataReader by its name."
        )
        .def(
            "find_registered_content_filters",
            [](pyrti::PyDomainParticipant& dp) {
                std::vector<std::string> v;
                rti::topic::find_registered_content_filters(dp, std::back_inserter(v));
                return v;
            },
            "Retrieve a list of all registered content filter names."
        )
        .def_property_readonly(
            "participant_reader",
            [](pyrti::PyDomainParticipant& dp) {
                std::vector<pyrti::PyDataReader<dds::topic::ParticipantBuiltinTopicData>> v;
                dds::sub::find<pyrti::PyDataReader<dds::topic::ParticipantBuiltinTopicData>>
                    (dds::sub::builtin_subscriber(dp), dds::topic::participant_topic_name(), std::back_inserter(v));
                if (v.size() == 0) throw dds::core::Error("Unable to retrieve built-in topic reader.");
                return v[0];
            },
            "Get the DomainParticipant built-in topic reader."
        )
        .def_property_readonly(
            "publication_reader",
            [](pyrti::PyDomainParticipant& dp) {
                std::vector<pyrti::PyDataReader<dds::topic::PublicationBuiltinTopicData>> v;
                dds::sub::find<pyrti::PyDataReader<dds::topic::PublicationBuiltinTopicData>>
                    (dds::sub::builtin_subscriber(dp), dds::topic::publication_topic_name(), std::back_inserter(v));
                if (v.size() == 0) throw dds::core::Error("Unable to retrieve built-in topic reader.");
                return v[0];
            },
            "Get the publication built-in topic reader."
        )
        .def_property_readonly(
            "subscription_reader",
            [](pyrti::PyDomainParticipant& dp) {
                std::vector<pyrti::PyDataReader<dds::topic::SubscriptionBuiltinTopicData>> v;
                dds::sub::find<pyrti::PyDataReader<dds::topic::SubscriptionBuiltinTopicData>>
                    (dds::sub::builtin_subscriber(dp), dds::topic::subscription_topic_name(), std::back_inserter(v));
                if (v.size() == 0) throw dds::core::Error("Unable to retrieve built-in topic reader.");
                return v[0];
            },
            "Get the subscription built-in topic reader."
        )
        .def_property_readonly(
            "topic_reader",
            [](pyrti::PyDomainParticipant& dp) {
                std::vector<pyrti::PyDataReader<dds::topic::TopicBuiltinTopicData>> v;
                dds::sub::find<pyrti::PyDataReader<dds::topic::TopicBuiltinTopicData>>
                    (dds::sub::builtin_subscriber(dp), dds::topic::topic_topic_name(), std::back_inserter(v));
                if (v.size() == 0) throw dds::core::Error("Unable to retrieve built-in topic reader.");
                return v[0];
            },
            "Get the topic built-in topic reader."
        )
        .def_property_readonly(
            "service_request_reader",
            [](pyrti::PyDomainParticipant& dp) {
                std::vector<pyrti::PyDataReader<rti::topic::ServiceRequest>> v;
                dds::sub::find<pyrti::PyDataReader<rti::topic::ServiceRequest>>
                    (dds::sub::builtin_subscriber(dp), rti::topic::service_request_topic_name(), std::back_inserter(v));
                if (v.size() == 0) throw dds::core::Error("Unable to retrieve built-in topic reader.");
                return v[0];
            },
            "Get the ServiceRequest built-in topic reader."
        )
        .def(
            "discovered_participants",
            [](pyrti::PyDomainParticipant& dp) {
                return rti::domain::discovered_participants(dp);
            },
            "Retrieves the instance handles of other DomainParticipants "
            "discovered by this one."
        )
        .def(
            "discovered_participant_data",
            [](pyrti::PyDomainParticipant& dp, const dds::core::InstanceHandle& handle) {
                return rti::domain::discovered_participant_data(dp, handle);
            },
            py::arg("handle"),
            "Retrieve DomainParticipant information by handle."
        )
        .def(
            "discovered_participant_data",
            [](pyrti::PyDomainParticipant& dp, const std::vector<dds::core::InstanceHandle>& handles) {
                std::vector<dds::topic::ParticipantBuiltinTopicData> v;
                for (auto& h : handles) {
                    v.push_back(rti::domain::discovered_participant_data(dp, h));
                }
                return v;
            },
            py::arg("handles"),
            "Retrieve DomainParticipant information with a sequence of "
            "handles."
        )
        .def(
            "publication_data",
            [](pyrti::PyDomainParticipant& dp, const dds::core::InstanceHandle& handle) {
                DDS_PublicationBuiltinTopicData pbitd;
                DDS_DomainParticipant_get_publication_data(
                    dp.delegate()->native_participant(), 
                    &pbitd,
                    &handle.delegate().native());
                dds::topic::PublicationBuiltinTopicData* ptr = 
                    reinterpret_cast<dds::topic::PublicationBuiltinTopicData*>(&pbitd);
                return *ptr;
            },
            py::arg("handle"),
            "Retrieve publication data by handle."
        )
        .def(
            "publication_data",
            [](pyrti::PyDomainParticipant& dp, const std::vector<dds::core::InstanceHandle>& handles) {
                std::vector<dds::topic::PublicationBuiltinTopicData> v;
                for (auto& handle : handles) {
                    DDS_PublicationBuiltinTopicData pbitd;
                    DDS_DomainParticipant_get_publication_data(
                    dp.delegate()->native_participant(), 
                    &pbitd,
                    &handle.delegate().native());
                    dds::topic::PublicationBuiltinTopicData* ptr = 
                        reinterpret_cast<dds::topic::PublicationBuiltinTopicData*>(&pbitd);
                   v.push_back(*ptr);
                }
                return v; 
            },
            py::arg("handles"),
            "Retrieve publication data for a sequence of handles."
        )
        .def(
            "subscription_data",
            [](pyrti::PyDomainParticipant& dp, const dds::core::InstanceHandle& handle) {
                DDS_SubscriptionBuiltinTopicData sbitd;
                DDS_DomainParticipant_get_subscription_data(
                    dp.delegate()->native_participant(), 
                    &sbitd,
                    &handle.delegate().native());
                dds::topic::SubscriptionBuiltinTopicData* ptr = 
                    reinterpret_cast<dds::topic::SubscriptionBuiltinTopicData*>(&sbitd);
                return *ptr;
            },
            py::arg("handle"),
            "Retrieve subscription data by handle."
        )
        .def(
            "subscription_data",
            [](pyrti::PyDomainParticipant& dp, const std::vector<dds::core::InstanceHandle>& handles) {
                std::vector<dds::topic::SubscriptionBuiltinTopicData> v;
                for (auto& handle : handles) {
                    DDS_SubscriptionBuiltinTopicData sbitd;
                    DDS_DomainParticipant_get_subscription_data(
                    dp.delegate()->native_participant(), 
                    &sbitd,
                    &handle.delegate().native());
                    dds::topic::SubscriptionBuiltinTopicData* ptr = 
                        reinterpret_cast<dds::topic::SubscriptionBuiltinTopicData*>(&sbitd);
                   v.push_back(*ptr);
                }
                return v; 
            },
            py::arg("handles"),
            "Retrieve subscription data for a sequence of handles."
        )
        .def(
            py::self == py::self,
            "Test for equality."
        )
        .def(
            py::self != py::self,
            "Test for inequality."
        )
        .doc() = "Container for all Entity objects.\n"
              "* It acts as a container for all other Entity objects.\n"
              "* It acts as a factory for the Publisher, Subscriber, Topic and Entity objects.\n"
              "* It represents the participation of the application on a communication plane that isolates applications running on the same set of physical computers from each other. A domain establishes a virtual network linking all applications that share the same domainId and isolating them from applications running on different domains. In this way, several independent distributed applications can coexist in the same physical network without interfering, or even being aware of each other.\n"
              "* It provides administration services in the domain, offering operations that allow the application to ignore locally any information about a given participant (DomainParticipant.ignore), publication (ignore_publication), subscription (ignore_subscription) or topic (ignore_topic).";
}

template<>
void pyrti::process_inits<DomainParticipant>(py::module& m, pyrti::ClassInitList& l) {
    l.push_back(
        [m]() mutable {
            return pyrti::init_class<pyrti::PyDomainParticipant, pyrti::PyIEntity>(m, "DomainParticipant");
        }
    ); 
}
