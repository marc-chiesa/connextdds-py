#pragma once

#include "PyConnext.hpp"
#include <pybind11/stl_bind.h>
#include <pybind11/operators.h>
#include <dds/sub/DataReader.hpp>
#include <rti/sub/AckResponseData.hpp>
#include <dds/sub/cond/ReadCondition.hpp>
#include <dds/sub/cond/QueryCondition.hpp>
#include <dds/sub/Query.hpp>
#include <dds/sub/find.hpp>
#include "PyEntity.hpp"
#include "PyCondition.hpp"
#include "PyAnyDataReader.hpp"
#include "PyTopic.hpp"
#include "PyDataReaderListener.hpp"
#include "PyContentFilteredTopic.hpp"
#include "PyDynamicTypeMap.hpp"

namespace pyrti {
    template<typename T>
    class PyDataReaderListener;

    class PyIDataReader : public pyrti::PyIAnyDataReader, public pyrti::PyIEntity {
    public:
        virtual
        dds::sub::Query create_query(const std::string&) = 0;

        virtual
        dds::sub::Query create_query(const std::string&, const std::vector<std::string>&) = 0; 
    };

    template<typename T>
    class PyDataReader :    public dds::sub::DataReader<T>, 
                            public PyIDataReader {
    public:
        using dds::sub::DataReader<T>::DataReader;

        /*PyDataReader(
            const pyrti::PySubscriber& s, 
            const pyrti::PyTopic<T>& t) : dds::sub::DataReader<T>(s, t) {}

        PyDataReader(
            const pyrti::PySubscriber& s, 
            const pyrti::PyTopic<T>& t, 
            const dds::sub::qos::DataReaderQos& q,
            pyrti::PyDataReaderListener<T>* l,
            const dds::core::status::StatusMask& m) : dds::sub::DataReader<T>(s, t, q, l, m) {}

        PyDataReader(
            const pyrti::PySubscriber& s, 
            const pyrti::PyContentFilteredTopic<T>& t) : dds::sub::DataReader<T>(s, t) {}

        PyDataReader(
            const pyrti::PySubscriber& s, 
            const pyrti::PyContentFilteredTopic<T>& t, 
            const dds::sub::qos::DataReaderQos& q,
            pyrti::PyDataReaderListener<T>* l,
            const dds::core::status::StatusMask& m) : dds::sub::DataReader<T>(s, t, q, l, m) {} */

        dds::core::Entity get_entity() override {
            return dds::core::Entity(*this);
        }

        dds::sub::AnyDataReader get_any_datareader() const override {
            return dds::sub::AnyDataReader(*this);
        }

        void py_enable() override { this->enable(); }

        const dds::core::status::StatusMask py_status_changes() override { return this->status_changes(); }

        const dds::core::InstanceHandle py_instance_handle() const override { return this->instance_handle(); }

        dds::sub::qos::DataReaderQos py_qos() const override { return this->qos(); }

        void py_qos(const dds::sub::qos::DataReaderQos& q) override { return this->qos(q); }

        const std::string& py_topic_name() const override { return this->delegate()->topic_name(); }

        const std::string& py_type_name() const override { return this->delegate()->type_name(); }

        const pyrti::PySubscriber py_subscriber() const override {
            auto s = this->subscriber();
            return pyrti::PySubscriber(s);
        }

        void py_close() override { this->close(); }

        void py_retain() override { this->retain(); }

        dds::sub::Query create_query(const std::string& expression) override {
            return dds::sub::Query(*this, expression);
        }

        dds::sub::Query create_query(const std::string& expression, const std::vector<std::string>& params) override {
            return dds::sub::Query(*this, expression, params);
        } 
    };

    template<typename T>
    void init_dds_typed_datareader_base_template(py::class_<pyrti::PyDataReader<T>, pyrti::PyIDataReader>& cls) {
        cls
            .def(
                py::init<
                    const pyrti::PySubscriber&,
                    const pyrti::PyTopic<T>&>(),
                py::arg("sub"),
                py::arg("topic"),
                "Create a DataReader."
            )
            .def(
                py::init<
                    const pyrti::PySubscriber&,
                    const pyrti::PyTopic<T>&,
                    const dds::sub::qos::DataReaderQos&,
                    pyrti::PyDataReaderListener<T>*, 
                    const dds::core::status::StatusMask&>(),
                py::arg("sub"),
                py::arg("topic"),
                py::arg("qos"),
                py::arg("listener") = (pyrti::PyDataReaderListener<T>*) nullptr,
                py::arg_v("mask", dds::core::status::StatusMask::all(), "StatusMask.all"),
                py::keep_alive<1,5>(),
                "Create a DataReader."
            )
            .def(
                py::init<
                    const pyrti::PySubscriber&,
                    const pyrti::PyContentFilteredTopic<T>&>(),
                py::arg("sub"),
                py::arg("cft"),
                "Create a DataReader with a ContentFilteredTopic."
            )
            .def(
                py::init<
                    const pyrti::PySubscriber&,
                    const pyrti::PyContentFilteredTopic<T>&,
                    const dds::sub::qos::DataReaderQos&,
                    pyrti::PyDataReaderListener<T>*, 
                    const dds::core::status::StatusMask&>(),
                py::arg("sub"),
                py::arg("cft"),
                py::arg("qos"),
                py::arg("listener") = (pyrti::PyDataReaderListener<T>*) nullptr,
                py::arg_v("mask", dds::core::status::StatusMask::all(), "StatusMask.all"),
                py::keep_alive<1,5>(),
                "Create a DataReader with a ContentFilteredTopic."
            )
            .def(
                py::init([](pyrti::PyIAnyDataReader& adr) {
                    auto dr = adr.get_any_datareader().get<T>();
                    return pyrti::PyDataReader<T>(dr);
                }),
                py::arg("reader"),
                "Create a typed DataReader from an AnyDataReader."
            )
            .def(
                py::init([](pyrti::PyIEntity& e) {
                    auto entity = e.get_entity();
                    return dds::core::polymorphic_cast<pyrti::PyDataReader<T>>(entity);
                }),
                py::arg("entity"),
                "Create a typed DataReader from an Entity."
            )
            .def_property(
                "default_filter_state",
                (const dds::sub::status::DataState& (pyrti::PyDataReader<T>::*)()) &pyrti::PyDataReader<T>::default_filter_state,
                [](pyrti::PyDataReader<T>& dr, const dds::sub::status::DataState& ds) {
                    dr.default_filter_state(ds);
                    return dr;
                },
                "Returns the filter state for the read/take operations."
            )
            .def(
                py::self == py::self,
                "Test for equality."
            )
            .def(
                py::self != py::self,
                "Test for inequality."
            )
            .def(
                "read",
                (dds::sub::LoanedSamples<T> (pyrti::PyDataReader<T>::*)()) &pyrti::PyDataReader<T>::read,
                "Read all samples using the default filter state"
            )
            .def(
                "read_valid",
                [](pyrti::PyDataReader<T>& dr) {
                    return rti::sub::ValidLoanedSamples<T>(dr.read());;
                },
                "Read only valid samples."
            )
            .def(
                "take",
                (dds::sub::LoanedSamples<T> (pyrti::PyDataReader<T>::*)()) &pyrti::PyDataReader<T>::take,
                "Take all samples using the default filter state"
            )
            .def(
                "take_valid",
                [](pyrti::PyDataReader<T>& dr) {
                    return rti::sub::ValidLoanedSamples<T>(dr.take());;
                },
                "Take only valid samples."
            )
            .def(
                "select",
                &pyrti::PyDataReader<T>::select,
                "Get a Selector to perform complex data selections, such as "
                "per-instance selection, content, and status filtering."
            )
            .def_property_readonly(
                "topic_description",
                &pyrti::PyDataReader<T>::topic_description,
                "Returns the TopicDescription associated with the DataReader."
            )
            .def_property_readonly(
                "subscriber",
                &pyrti::PyDataReader<T>::py_subscriber,
                "Returns the parent Subscriber of the DataReader."
            )
            .def_property_readonly(
                "listener",
                [](pyrti::PyDataReader<T>& dr) {
                    return dynamic_cast<pyrti::PyDataReaderListener<T>*>(dr.listener());
                },
                "Get the listener object."
            )
            .def(
                "bind_listener",
                [](pyrti::PyDataReader<T>& dr, pyrti::PyDataReaderListener<T>* l, const dds::core::status::StatusMask& m) {
                    dr.listener(l, m);
                },
                py::arg("listener"),
                py::arg("event_mask"),
                py::keep_alive<1, 2>(),
                "Set the listener and associated event mask."
            )
            .def_property(
                "qos",
                (dds::sub::qos::DataReaderQos (pyrti::PyDataReader<T>::*)() const) &pyrti::PyDataReader<T>::qos,
                (void (pyrti::PyDataReader<T>::*)(const dds::sub::qos::DataReaderQos&)) &pyrti::PyDataReader<T>::qos,
                "Get a copy of or set the DataReaderQos"
            )
            .def(
                "__lshift__",
                [](pyrti::PyDataReader<T>& dr, const dds::sub::qos::DataReaderQos& qos) {
                    return (dr << qos);
                },
                py::is_operator(),
                "Set the DataReaderQos for this DataReader."
            )
            .def(
                "__rshift__",
                [](pyrti::PyDataReader<T>& dr, dds::sub::qos::DataReaderQos& qos) {
                    return (dr >> qos);
                },
                py::is_operator(),
                "Get the DataReaderQos from this DataReader"
            )
            .def_property_readonly(
                "liveliness_changed_status",
                &pyrti::PyDataReader<T>::liveliness_changed_status,
                "Get the LivelinessChangedStatus for this DataReader."
            )
            .def_property_readonly(
                "sample_rejected_status",
                &pyrti::PyDataReader<T>::sample_rejected_status,
                "Get the SampleRejectedStatus for the DataReader."
            )
            .def_property_readonly(
                "sample_lost_status",
                &pyrti::PyDataReader<T>::sample_lost_status,
                "Get the SampleLostStatus for the DataReader."
            )
            .def_property_readonly(
                "requested_deadline_missed_status",
                &pyrti::PyDataReader<T>::requested_deadline_missed_status,
                "Get the RequestedDeadlineMissed status for the DataReader"
            )
            .def_property_readonly(
                "requested_incompatible_qos_status",
                &pyrti::PyDataReader<T>::requested_incompatible_qos_status,
                "Get the RequestedIncompatibleQosStatus for the DataReader."
            )
            .def_property_readonly(
                "subscription_matched_status",
                &pyrti::PyDataReader<T>::subscription_matched_status,
                "Get the SubscriptionMatchedStatus for the DataReader."
            )
            .def_property_readonly(
                "datareader_cache_status",
                [](pyrti::PyDataReader<T>& dr){
                    return dr->datareader_cache_status();
                },
                "Get the DataReaderCacheStatus for the DataReader."
            )
            .def_property_readonly(
                "datareader_protocol_status",
                [](pyrti::PyDataReader<T>& dr) {
                    return dr->datareader_protocol_status();
                },
                "Get the DataReaderProtocolStatus for the DataReader."
            )
            .def(
                "matched_publication_datareader_protocol_status",
                [](pyrti::PyDataReader<T>& dr, const dds::core::InstanceHandle& handle) {
                    return dr->matched_publication_datareader_protocol_status(handle);
                },
                py::arg("publication_handle"),
                "Get the DataReaderProtocolStatus for the DataReader based on the "
                "matched publication handle."
            )
            .def(
                "acknowledge_all",
                [](pyrti::PyDataReader<T>& dr) {
                    dr->acknowledge_all();
                },
                "Acknowledge all previously accessed samples."
            )
            .def(
                "acknowledge_all",
                [](pyrti::PyDataReader<T>& dr, const rti::sub::AckResponseData& rd) {
                    dr->acknowledge_all(rd);
                },
                "Acknowledge all previously accessed samples."
            )
            .def(
                "acknowledge_sample",
                [](pyrti::PyDataReader<T>& dr, const dds::sub::SampleInfo& info) {
                    dr->acknowledge_sample(info);
                },
                py::arg("sample_info"),
                "Acknowledge a single sample."
            )
            .def(
                "is_data_consistent",
                [](pyrti::PyDataReader<T>& dr, const T& data, const dds::sub::SampleInfo& info) {
                    return dr->is_data_consistent(data, info);
                },
                py::arg("sample_data"),
                py::arg("sample_info"),
                "Checks if the sample has been overwritten by the DataWriter."
            )
            .def(
                "is_data_consistent",
                [](pyrti::PyDataReader<T>& dr, const dds::sub::Sample<T>& sample) {
                    return dr->is_data_consistent(sample.data(), sample.info());
                },
                py::arg("sample"),
                "Checks if the sample has been overwritten by the DataWriter."
            )
            .def(
                "is_data_consistent",
                [](pyrti::PyDataReader<T>& dr, const rti::sub::LoanedSample<T>& sample) {
                    return dr->is_data_consistent(sample);
                },
                py::arg("sample"),
                "Checks if the sample has been overwritten by the DataWriter."
            )
            .def_property_readonly(
                "topic_name",
                [](pyrti::PyDataReader<T>& dr) {
                    return dr->topic_name();
                },
                "Get the topic name associated with this DataReader."
            )
            .def_property_readonly(
                "type_name",
                [](pyrti::PyDataReader<T>& dr) {
                    return dr->type_name();
                },
                "Get the type name associated with this DataReader."
            )
            .def(
                "close",
                [](pyrti::PyDataReader<T>& dr) {
                    dr->close();
                },
                "Close this DataReader."
            )
            .def(
                "__enter__",
                [](pyrti::PyDataReader<T>& dr) {
                    return dr;
                },
                "Enter a context for this DataReader, to be cleaned up on "
                "exiting context"
            )
            .def(
                "__exit__",
                [](pyrti::PyDataReader<T>& dr, py::object, py::object, py::object) {
                    dr->close();
                },
                "Exit the context for this DataReader, cleaning up resources."
            )
            .def_static(
                "find_all_by_topic",
                [](pyrti::PySubscriber& sub, const std::string& topic_name) {
                    std::vector<pyrti::PyDataReader<T>> v;
                    dds::sub::find<pyrti::PyDataReader<T>>(sub, topic_name, std::back_inserter(v));
                    return v;
                },
                py::arg("subscriber"),
                py::arg("topic_name"),
                "Retrieve all DataReaders for the given topic name in the subscriber."
            )
            .def_static(
                "find_by_name",
                [](pyrti::PySubscriber& sub, const std::string& name) -> py::object {
                    auto dr = rti::sub::find_datareader_by_name<pyrti::PyDataReader<T>>(sub, name);
                    return (dr == dds::core::null) ? py::cast(nullptr) : py::cast(dr);
                },
                py::arg("subscriber"),
                py::arg("name"),
                "Find DataReader in Subscriber with the DataReader's name, "
                "returning the first found."
            )
            .def_static(
                "find_by_name",
                [](pyrti::PyDomainParticipant& dp, const std::string name) ->py::object {
                    auto dr = rti::sub::find_datareader_by_name<pyrti::PyDataReader<T>>(dp, name);
                    return (dr == dds::core::null) ? py::cast(nullptr) : py::cast(dr);
                },
                py::arg("participant"),
                py::arg("name"),
                "Find DataReader in DomainParticipant with the DataReader's "
                "name, returning the first found."
            )
            .def_static(
                "find_by_topic",
                [](pyrti::PySubscriber& sub, const std::string& topic_name) -> py::object {
                    auto dr = rti::sub::find_datareader_by_topic_name<pyrti::PyDataReader<T>>(sub, topic_name);
                    return (dr == dds::core::null) ? py::cast(nullptr) : py::cast(dr);
                },
                py::arg("subscriber"),
                py::arg("name"),
                "Find DataReader in Subscriber with a topic name, "
                "returning the first found."
            );

        py::class_<typename pyrti::PyDataReader<T>::Selector>(cls, "Selector")
            .def(
                py::init<
                    pyrti::PyDataReader<T>&
                >(),
                py::arg("datareader"),
                "Create a Selector for a DataReader to read/take based on "
                "selected conditions"
            )
            .def(
                "instance",
                &pyrti::PyDataReader<T>::Selector::instance,
                py::arg("handle"),
                "Select a specific instance to read/take."
            )
            .def(
                "next_instance",
                &pyrti::PyDataReader<T>::Selector::next_instance,
                py::arg("previous"),
                "Select the instance after the specified instance to read/take."
            )
            .def(
                "state",
                &pyrti::PyDataReader<T>::Selector::state,
                py::arg("state"),
                "Select samples with a specified data state."
            )
            .def(
                "content",
                &pyrti::PyDataReader<T>::Selector::content,
                py::arg("query"),
                "Select samples based on a Query."
            )
            .def(
                "condition",
                [](typename pyrti::PyDataReader<T>::Selector& sel, pyrti::PyIReadCondition& rc) {
                    auto cond = rc.get_read_condition();
                    return sel.condition(cond);
                },
                py::arg("condition"),
                "Select samples based on a ReadCondition."
            )
            .def(
                "max_samples",
                &pyrti::PyDataReader<T>::Selector::max_samples,
                py::arg("max"),
                "Limit the number of samples read/taken by the Select."
            )
            .def(
                "read",
                (dds::sub::LoanedSamples<T> (pyrti::PyDataReader<T>::Selector::*)()) &pyrti::PyDataReader<T>::Selector::read,
                "Read samples based on Selector settings."
            )
            .def(
                "read_valid",
                [](typename pyrti::PyDataReader<T>::Selector& s) {
                    return rti::sub::ValidLoanedSamples<T>(s.read());
                },
                "Read valid samples based on Selector settings."
            )
            .def(
                "take",
                (dds::sub::LoanedSamples<T> (pyrti::PyDataReader<T>::Selector::*)()) &pyrti::PyDataReader<T>::Selector::take,
                "Take samples based on Selector settings."
            )
            .def(
                "take_valid",
                [](typename pyrti::PyDataReader<T>::Selector& s) {
                    return rti::sub::ValidLoanedSamples<T>(s.take());
                },
                "Take valid samples based on Selector settings."
            );

        py::implicitly_convertible<pyrti::PyIAnyDataReader, pyrti::PyDataReader<T>>();
        py::implicitly_convertible<pyrti::PyIEntity, pyrti::PyDataReader<T>>();

        py::bind_vector<std::vector<pyrti::PyDataReader<T>>>(cls, "Seq");
        py::implicitly_convertible<py::iterable, std::vector<pyrti::PyDataReader<T>>>();
    }

    template<typename T>
    void init_dds_typed_datareader_template(py::class_<pyrti::PyDataReader<T>, pyrti::PyIDataReader>& cls) {
        init_dds_typed_datareader_base_template<T>(cls);

        cls
            .def(
                "key_value",
                [](pyrti::PyDataReader<T>& dr, const dds::core::InstanceHandle& handle) {
                    T d;
                    dr.key_value(d, handle);
                    return d;
                },
                py::arg("handle"),
                "Retrieve the instance key that corresponds to an instance handle."
            )
            .def(
                "topic_instance_key_value",
                [](pyrti::PyDataReader<T>& dr, const dds::core::InstanceHandle& handle) {
                    T d;
                    dds::topic::TopicInstance<T> ti(handle, d);
                    dr.key_value(ti, handle);
                    return ti;
                },
                py::arg("handle"),
                "Retrieve the instance key that corresponds to an instance handle."
            )
            .def(
                "read_next",
                [](pyrti::PyDataReader<T>& dr) -> py::object {
                    T data;
                    dds::sub::SampleInfo info;
                    if (dr->read(data, info)) {
                        return py::cast(dds::sub::Sample<T>(data, info));
                    }
                    return py::cast(nullptr);
                },
                "Copy the next not-previously-accessed data value from the "
                "DataReader via a read operation."
            )
            .def(
                "take_next",
                [](pyrti::PyDataReader<T>& dr) -> py::object{
                    T data;
                    dds::sub::SampleInfo info;
                    if (dr->take(data, info)) {
                        return py::cast(dds::sub::Sample<T>(data, info));
                    }
                    return py::cast(nullptr);
                },
                "Copy the next not-previously-accessed data value from the "
                "DataReader via a take operation."
            );
    }

    template<typename T>
    void init_datareader(py::object& o) {
        py::class_<pyrti::PyDataReader<T>, pyrti::PyIDataReader> dr(o, "DataReader");
        
        init_dds_typed_datareader_template(dr);
    }
}