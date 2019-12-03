/*
    MIT License

    Copyright (c) 2019 Marco Melorio

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/

#pragma once

#include <pybind11/embed.h>

namespace py = pybind11;

class PyStdErrOutStreamRedirect {
    public:
        PyStdErrOutStreamRedirect() {
            auto sysm = py::module::import("sys");
            _stdout = sysm.attr("stdout");
            _stderr = sysm.attr("stderr");

            auto stringio = py::module::import("io").attr("StringIO");
            _stdoutBuffer = stringio();
            _stderrBuffer = stringio();
            sysm.attr("stdout") = _stdoutBuffer;
            sysm.attr("stderr") = _stderrBuffer;
        }

        ~PyStdErrOutStreamRedirect() {
            auto sysm = py::module::import("sys");
            sysm.attr("stdout") = _stdout;
            sysm.attr("stderr") = _stderr;
        }

        std::string stdoutString() {
            _stdoutBuffer.attr("seek")(0);
            std::string pyOut = py::str(_stdoutBuffer.attr("read")());

            auto stringio = py::module::import("io").attr("StringIO");
            _stdoutBuffer = stringio();

            auto sysm = py::module::import("sys");
            sysm.attr("stdout") = _stdoutBuffer;

            return pyOut;
        }

        std::string stderrString() {
            _stderrBuffer.attr("seek")(0);
            std::string pyErr = py::str(_stderrBuffer.attr("read")());

            auto stringio = py::module::import("io").attr("StringIO");
            _stderrBuffer = stringio();

            auto sysm = py::module::import("sys");
            sysm.attr("stderr") = _stderrBuffer;

            return pyErr;
        }

    private:
        py::object _stdout;
        py::object _stderr;
        py::object _stdoutBuffer;
        py::object _stderrBuffer;
};
