/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
   University of Utah.

   License for the specific language governing rights and limitations under
   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.
*/


#ifndef DATAFLOW_NETWORK_PORT_INTERFACE_H
#define DATAFLOW_NETWORK_PORT_INTERFACE_H 

#include <string>
#include <Dataflow/Network/NetworkFwd.h>
#include <Core/Datatypes/Datatype.h>
#include <Dataflow/Network/Share.h>

namespace SCIRun {
namespace Dataflow {
namespace Networks {

  class SCISHARE PortInterface
  {
  public:
    virtual ~PortInterface();
    virtual void attach(Connection* conn) = 0;
    virtual void detach(Connection* conn) = 0;
    virtual size_t nconnections() const = 0;
    virtual const Connection* connection(size_t) const = 0;
    virtual std::string get_colorname() const = 0;
    virtual std::string get_portname() const = 0;

    virtual void reset() = 0;
    virtual void finish() = 0;
  };
  
  class SCISHARE InputPortInterface : virtual public PortInterface
  {
  public:
    virtual ~InputPortInterface();
    virtual Core::Datatypes::DatatypeHandleOption getData() = 0;
    virtual DatatypeSinkInterfaceHandle sink() = 0;
  };
  
  class SCISHARE OutputPortInterface : virtual public PortInterface
  {
  public:
    virtual ~OutputPortInterface();
    virtual void sendData(Core::Datatypes::DatatypeHandle data) = 0;
  };
}}}

#endif
