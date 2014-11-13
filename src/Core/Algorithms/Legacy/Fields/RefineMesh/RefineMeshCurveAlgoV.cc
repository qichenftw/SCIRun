/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2009 Scientific Computing and Imaging Institute,
   University of Utah.

   
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


#include <Core/Algorithms/Legacy/Fields/RefineMesh/RefineMesh.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h> 
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/Matrix.h>
// For mapping matrices
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/FieldInformation.h>
#include <Core/Algorithms/Base/AlgorithmPrecondition.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>

//STL classes needed
#include <sci_hash_map.h>
#include <algorithm>
#include <set>

ALGORITHM_PARAMETER_DEF(Fields, RefineMethod);
ALGORITHM_PARAMETER_DEF(Fields, AddConstraints);
ALGORITHM_PARAMETER_DEF(Fields, IsoValue);



///////////////////////////////////////////////////////
// Refine elements for a CurveMesh

bool  
RefineMeshCurveAlgoV(AlgoBase* algo, FieldHandle input, FieldHandle& output,
                       std::string select, double isoval)
{
  /// Obtain information on what type of input field we have
  FieldInformation fi(input);
  
  /// Alter the input so it will become a QuadSurf
  fi.make_curvemesh();
  output = CreateField(fi);
  
  if (output.get_rep() == 0)
  {
    algo->error("RefineMesh: Could not create an output field");
    algo->algo_end(); return (false);
  }

  VField* field   = input->vfield();
  VMesh*  mesh    = input->vmesh();
  VMesh*  refined = output->vmesh();
  VField* rfield  = output->vfield();

  VMesh::Node::array_type onodes(2);
  
  // get all values, make computation easier
  VMesh::size_type num_nodes = mesh->num_nodes();
  VMesh::size_type num_elems = mesh->num_elems();
  
  std::vector<bool> values(num_nodes,false);
 
  // Deal with data stored at different locations
  // If data is on the elements make sure that all nodes
  // of that element pass requirement.

  std::vector<double> ivalues;
  std::vector<double> evalues;
  
  if (field->basis_order() == 0)
  {
    field->get_values(ivalues);
    
    if (select == "equal")
    {
      for (VMesh::Elem::index_type i=0; i<num_elems; i++)
      {
        mesh->get_nodes(onodes,i);
        if (ivalues[i] == isoval)
          for (size_t j=0; j< onodes.size(); j++)
            values[onodes[j]] = true;
      }
    }
    else if (select == "lessthan")
    {
      for (VMesh::Elem::index_type i=0; i<num_elems; i++)
      {
        mesh->get_nodes(onodes,i);
        if (ivalues[i] < isoval)
          for (size_t j=0; j< onodes.size(); j++)
            values[onodes[j]] = true;
      }    
    }
    else if (select == "greaterthan")
    {
      for (VMesh::Elem::index_type i=0; i<num_elems; i++)
      {
        mesh->get_nodes(onodes,i);
        if (ivalues[i] > isoval)
          for (size_t j=0; j< onodes.size(); j++)
            values[onodes[j]] = true;
      }    
    }
    else if (select == "all")
    {
      for (size_t j=0;j<values.size();j++) values[j] = true;
    }
    else
    {
      algo->error("RefineMesh: Unknown region selection method encountered");
      algo->algo_end(); return (false);
    }
  }
  else if (field->basis_order() == 1)
  {
    field->get_values(ivalues);

    if (select == "equal")
    {
      for (VMesh::Elem::index_type i=0; i<num_nodes; i++)
      {
        if (ivalues[i] == isoval) values[i] = true;
      }
    }
    else if (select == "lessthan")
    {
      for (VMesh::Elem::index_type i=0; i<num_nodes; i++)
      {
        if (ivalues[i] < isoval) values[i] = true;
      }    
    }
    else if (select == "greaterthan")
    {
      for (VMesh::Elem::index_type i=0; i<num_nodes; i++)
      {
        if (ivalues[i] > isoval) values[i] = true;
      }    
    }
    else if (select == "all")
    {
      for (size_t j=0;j<values.size();j++) values[j] = true;
    }    
    else
    {
      algo->error("RefineMesh: Unknown region selection method encountered");
      algo->algo_end(); return (false);
    }

  }
  else
  {
    for (size_t j=0;j<values.size();j++) values[j] = true;
  }
  
  // Copy all of the nodes from mesh to refined.  They won't change,
  // we only add nodes.
  
  VMesh::Node::iterator bni, eni;
  mesh->begin(bni); mesh->end(eni);
  while (bni != eni)
  {
    Point p;
    mesh->get_point(p, *bni);
    refined->add_point(p);
    ++bni;
  }

  std::vector<VMesh::index_type> enodes(mesh->num_edges(),0);

  VMesh::Node::array_type nodes(2), nnodes(2);
  Point p0, p1,p2, p3, p;
  VMesh::Elem::iterator be, ee;
  mesh->begin(be); mesh->end(ee);

  // add all additional nodes we need
  while (be != ee)
  {
    mesh->get_nodes(nodes,*be);
    if ((values[nodes[0]] == true) || (values[nodes[1]] == true))
    {
      mesh->get_center(p0,nodes[0]);
      mesh->get_center(p1,nodes[1]);
    
      p = (p0.asVector() + p1.asVector()).asPoint()*0.5;
      enodes[*be] = refined->add_point(p);

      nnodes[0] = nodes[0];
      nnodes[1] = enodes[*be];
      refined->add_elem(nnodes);

      nnodes[0] = enodes[*be];
      nnodes[1] = nodes[1];
      refined->add_elem(nnodes);

      if (field->basis_order() == 1) 
        ivalues.push_back(0.5*(ivalues[nodes[0]]+ivalues[nodes[1]]));
      else if (field->basis_order() == 0) 
	evalues.insert(evalues.end(),2,ivalues[*be]); 
    }
    ++be;
  }

  rfield->resize_values();
  if (rfield->basis_order() == 0) rfield->set_values(evalues);
  if (rfield->basis_order() == 1) rfield->set_values(ivalues);
  rfield->copy_properties(field);

  algo->algo_end(); return (true);
}

    