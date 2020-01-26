#include "PyConnext.hpp"
#include "PyLogger.hpp"
#include <mutex>

namespace pyrti {
    bool PyLogger::_options_set = false;
    std::recursive_mutex PyLogger::_lock;
    std::unique_ptr<PyLogger> PyLogger::_py_instance;

    PyLogger& PyLogger::instance() {
        std::lock_guard<std::recursive_mutex> lock(PyLogger::_lock);

        if (!PyLogger::_py_instance) {
            if (!PyLogger::_options_set) {
                PyLogger::options(PyLoggerOptions());
                PyLogger::_options_set = true;
            }
            PyLogger::_py_instance.reset(new PyLogger());
        }
        return *PyLogger::_py_instance;
    }

    bool PyLogger::options(const PyLoggerOptions& options) {
        std::lock_guard<std::recursive_mutex> lock(PyLogger::_lock);

        bool retval = (bool)RTI_DL_DistLogger_setOptions(options._options);

        if (retval) {
            PyLogger::_options_set = true;
        }

        return retval;
    }

    void PyLogger::finalize() {
        std::lock_guard<std::recursive_mutex> lock(PyLogger::_lock);
        if (PyLogger::_py_instance == nullptr) {
            return;
        }

        auto ptr = PyLogger::_py_instance.release();
        delete ptr;
    }

    PyLogger::PyLogger() {
        this->_instance = RTI_DL_DistLogger_getInstance();
    }

    PyLogger::~PyLogger() {
        RTI_DL_DistLogger_finalizeInstance();
    }

    PyLogger& PyLogger::filter_level(PyLogLevel level) {
        auto retval = RTI_DL_DistLogger_setFilterLevel(this->_instance, (int)level);
        if (retval != DDS_RETCODE_OK) throw dds::core::Error("Could not set Distributed Logger filter level");
        return *this;
    }

    PyLogger& PyLogger::print_format(const rti::config::PrintFormat& level) {
        auto retval = RTI_DL_DistLogger_setRTILoggerPrintFormat(this->_instance, (NDDS_Config_LogPrintFormat)level.underlying());
        if (retval != RTI_TRUE) throw dds::core::Error("Could not set Distributed Logger filter level");
        return *this;
    }

    PyLogger& PyLogger::verbosity(const rti::config::LogCategory& category, const rti::config::Verbosity& level) {
        RTI_DL_DistLogger_setRTILoggerVerbosityByCategory(this->_instance, (NDDS_Config_LogCategory)category.underlying(), (NDDS_Config_LogVerbosity)level.underlying());
        return *this;
    }

    void PyLogger::log(PyLogLevel level, const std::string& message, const std::string& category) {
        RTI_DL_DistLogger_logMessageWithLevelCategory(
            this->_instance,
            (int)level,
            message.c_str(),
            category.c_str());
    }

    void PyLogger::log(const PyMessageParams& params) {
        RTI_DL_DistLogger_logMessageWithParams(
            this->_instance,
            &params._params
        );
    }

    void PyLogger::fatal(const std::string& message) {
        RTI_DL_DistLogger_fatal(this->_instance, message.c_str());
    }

    void PyLogger::severe(const std::string& message) {
        RTI_DL_DistLogger_severe(this->_instance, message.c_str());
    }

    void PyLogger::error(const std::string& message) {
        RTI_DL_DistLogger_error(this->_instance, message.c_str());
    }

    void PyLogger::warning(const std::string& message) {
        RTI_DL_DistLogger_warning(this->_instance, message.c_str());
    }

    void PyLogger::notice(const std::string& message) {
        RTI_DL_DistLogger_notice(this->_instance, message.c_str());
    }

    void PyLogger::info(const std::string& message) {
        RTI_DL_DistLogger_info(this->_instance, message.c_str());
    }

    void PyLogger::debug(const std::string& message) {
        RTI_DL_DistLogger_debug(this->_instance, message.c_str());
    }

    void PyLogger::trace(const std::string& message) {
        RTI_DL_DistLogger_trace(this->_instance, message.c_str());
    }

    void PyLogger::log(PyLogLevel level, const std::string& message) {
        RTI_DL_DistLogger_log(this->_instance, (int)level, message.c_str());
    }
}

void init_logger(py::module& m) {
    py::class_<pyrti::PyLogger>(m, "Logger")
        .def_property_readonly(
            "filter_level",
            &pyrti::PyLogger::filter_level,
            "The logger filter level."
        )
        .def_property_readonly(
            "print_format",
            &pyrti::PyLogger::print_format,
            "The logger print format."
        )
        .def_property_readonly(
            "verbosity",
            &pyrti::PyLogger::verbosity,
            "The logger's verbosity."
        )
        .def(
            "log",
            (void (pyrti::PyLogger::*)(pyrti::PyLogLevel, const std::string&)) &pyrti::PyLogger::log,
            py::arg("log_level"),
            py::arg("message"),
            "Log a message with the given log level"
        )
        .def(
            "log",
            (void (pyrti::PyLogger::*)(pyrti::PyLogLevel, const std::string&, const std::string&)) &pyrti::PyLogger::log,
            py::arg("log_level"),
            py::arg("message"),
            py::arg("category"),
            "Log a message with the given log level and category"
        )
        .def(
            "log",
            (void (pyrti::PyLogger::*)(const pyrti::PyMessageParams&)) &pyrti::PyLogger::log,
            py::arg("message_params"),
            "Log a message with the given log level and category"
        )
        .def(
            "fatal",
            &pyrti::PyLogger::fatal,
            "Log a fatal message."
        )
        .def(
            "severe",
            &pyrti::PyLogger::severe,
            "Log a severe message."
        )
        .def(
            "error",
            &pyrti::PyLogger::error,
            "Log an error message."
        )
        .def(
            "warning",
            &pyrti::PyLogger::warning,
            "Log a warning message."
        )
        .def(
            "notice",
            &pyrti::PyLogger::notice,
            "Log a notice message."
        )
        .def(
            "info",
            &pyrti::PyLogger::info,
            "Log an info message."
        )
        .def(
            "debug",
            &pyrti::PyLogger::debug,
            "Log a debug message."
        )
        .def(
            "trace",
            &pyrti::PyLogger::trace,
            "Log a trace message."
        )
        .def_property_readonly_static(
            "instance",
            [](py::object&) -> pyrti::PyLogger& {
                return pyrti::PyLogger::instance();
            },
            "Get the Logger instance."
        )
        .def_static(
            "options",
            &pyrti::PyLogger::options,
            py::arg("options"),
            "Set the options for the Logger instance (must be set prior to "
            "accessing the instance."
        )
        .def_static(
            "finalize",
            &pyrti::PyLogger::finalize,
            "Destroy the Logger. It should not be accessed after this call."
        );
}