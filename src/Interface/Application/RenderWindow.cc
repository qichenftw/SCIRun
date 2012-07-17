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

#include "RenderWindow.h"
#include "ui_RenderWindow.h"

#include <QVTKWidget.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkPolyDataMapper.h>
#include <vtkArrowSource.h>
#include <vtkVector.h>
#include <vtkMatrix3x3.h>
#include <vtkMatrix4x4.h>
#include <vtkTransform.h>

RenderWindow::RenderWindow(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::RenderWindow),
  mVtkWidget(new QVTKWidget(this, QFlag(0)))
{
  ui->setupUi(this);

  // Add VTK widget to vertical layout.
  ui->verticalLayout->addWidget(mVtkWidget);
  ui->verticalLayout->update();

  // Create renderer
  mRen = vtkSmartPointer<vtkRenderer>::New();
  mVtkWidget->GetRenderWindow()->AddRenderer(mRen);
  //mRen->SetBackground(1.0,0.0,0.0);

  // Create arrow poly data
  mArrowSource = vtkSmartPointer<vtkArrowSource>::New();
  //arrowSource->SetShaftRadius(1.0);
  //arrowSource->SetTipLength(1.0);
  mArrowSource->Update();

  // Create data mapper (from visualization system to graphics system).
  mArrowMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  mArrowMapper->SetInputConnection(mArrowSource->GetOutputPort());

  //// Create actor that will be displayed in the scene.
  //vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
  //actor->SetMapper(mapper);

  //// Add the actor to the renderer.
  //mRen->AddActor(actor);
  setupVectorField();
}


RenderWindow::~RenderWindow()
{
  mRen->Delete();
  delete mVtkWidget;
  delete ui;
}


void RenderWindow::setupVectorField()
{
  // Read matrix and build scene with appropriate actors.
  const int numVecs = 6;
  double vpd[3][numVecs] = { // Vector position data
    { 0.0, 0.0, 0.0, 0.0, 1.0, 1.0 },
    { 0.0, 1.0, 0.0, 1.0, 1.0, 0.0 },
    { 0.0, 0.0, 1.0, 1.0, 0.0, 1.0 }
  };
  double vod[3][numVecs] = { // Vector orientation data
    { 1.0, 0.0, 0.0, 0.0, 1.0, 1.0 },
    { 0.0, 1.0, 0.0, 1.0, 1.0, 0.0 },
    { 0.0, 0.0, 1.0, 1.0, 0.0, 1.0 }
  };
    
  // Setup actors for each of the vectors...
  // There are better ways of representing vectors fields in VTK.

  // Need to construct 3x3 rotation matrices from orientation vector.
  for (int i = 0; i < numVecs; i++)
  {
    //vtkSmartPointer<vtkMatrix4x4> orient = matFromVec(vtkVector3d(vod[0][i],
    //      vod[1][i], vod[2][i]));
    vtkSmartPointer<vtkMatrix4x4> orient = vtkSmartPointer<vtkMatrix4x4>::New();
    orient->Identity();
    std::cout << orient->GetReferenceCount() << std::endl;

    //// Populate right most vector with position data.
    //*orient[0][3] = vpd[0][i];
    //*orient[1][3] = vpd[1][i];
    //*orient[2][3] = vpd[2][i];

    std::cout << orient->GetReferenceCount() << std::endl;

    std::cout << " " << *orient[0][0] << " " << *orient[0][1] << " " << *orient[0][2] << " " << *orient[0][3] << std::endl;
    std::cout << " " << *orient[1][0] << " " << *orient[1][1] << " " << *orient[1][2] << " " << *orient[1][3] << std::endl;
    std::cout << " " << *orient[2][0] << " " << *orient[2][1] << " " << *orient[2][2] << " " << *orient[2][3] << std::endl;
    std::cout << " " << *orient[3][0] << " " << *orient[3][1] << " " << *orient[3][2] << " " << *orient[3][3] << std::endl;

    std::cout << orient->GetReferenceCount() << std::endl;

    // Now we have appropriate transformation data, build actor.
    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    std::cout << actor->GetReferenceCount() << std::endl;
    actor->SetMapper(mArrowMapper);
    std::cout << mArrowMapper->GetReferenceCount() << std::endl;
    std::cout << orient->GetReferenceCount() << std::endl;
    actor->SetUserMatrix(orient); // NOTE: Requesting a pointer!
                                  // Make it a smart pointer if it is.
    std::cout << orient->GetReferenceCount() << std::endl;
    mRen->AddActor(actor);
    std::cout << actor->GetReferenceCount() << std::endl;



  }


}

vtkSmartPointer<vtkMatrix4x4> RenderWindow::matFromVec(const vtkVector3d& v)
{
  // Find the maximal component in the vector. The construct a normal vector
  // which has a zero for the maximal component.
  float absX = fabs(v.GetX());
  float absY = fabs(v.GetY());
  float absZ = fabs(v.GetZ());

  vtkVector3d gen;
  if (absX > absY)
  {
    if (absZ > absX)
    {
      gen = vtkVector3d(1.0, 1.0, 0.0);
    }
    else
    {
      gen = vtkVector3d(0.0, 1.0, 1.0);
    }
  }
  else
  {
    if (absZ > absY)
    {
      gen = vtkVector3d(1.0, 1.0, 0.0);
    }
    else
    {
      gen = vtkVector3d(1.0, 0.0, 1.0);
    }
  }

  gen.Normalize();

  vtkVector3d at = v;

  // Cross generated vector with 'at' vector.
  vtkVector3d right = gen.Cross(at);
  right.Normalize(); // Only need to do this if at is non-normal.
  vtkVector3d up = at.Cross(right);
  up.Normalize();

  vtkSmartPointer<vtkMatrix4x4> r = vtkSmartPointer<vtkMatrix4x4>::New();
  vtkMatrix4x4::Identity(&r->Element[0][0]);
  r->Identity();

  *r[0][0] = 1.0; *r[0][1] = 0.0; *r[0][2] = 0.0; *r[0][3] = 0.0;
  *r[1][0] = 0.0; *r[1][1] = 1.0; *r[1][2] = 0.0; *r[1][3] = 0.0;
  *r[2][0] = 0.0; *r[2][1] = 0.0; *r[2][2] = 1.0; *r[2][3] = 0.0;
  *r[3][0] = 0.0; *r[3][1] = 0.0; *r[3][2] = 0.0; *r[3][3] = 1.0;

  *r[0][0] = right[0]; *r[0][1] = up   [0]; *r[0][2] = at   [0];

  //std::cout << " " << *r[0][0] << " " << *r[0][1] << " " << *r[0][2] << " " << *r[0][3] << std::endl;
  //std::cout << " " << *r[1][0] << " " << *r[1][1] << " " << *r[1][2] << " " << *r[1][3] << std::endl;
  //std::cout << " " << *r[2][0] << " " << *r[2][1] << " " << *r[2][2] << " " << *r[2][3] << std::endl;
  //std::cout << " " << *r[3][0] << " " << *r[3][1] << " " << *r[3][2] << " " << *r[3][3] << std::endl;

  *r[1][0] = right[1]; *r[1][1] = up   [1]; *r[1][2] = at   [1];

  //std::cout << " " << *r[0][0] << " " << *r[0][1] << " " << *r[0][2] << " " << *r[0][3] << std::endl;
  //std::cout << " " << *r[1][0] << " " << *r[1][1] << " " << *r[1][2] << " " << *r[1][3] << std::endl;
  //std::cout << " " << *r[2][0] << " " << *r[2][1] << " " << *r[2][2] << " " << *r[2][3] << std::endl;
  //std::cout << " " << *r[3][0] << " " << *r[3][1] << " " << *r[3][2] << " " << *r[3][3] << std::endl;

  *r[2][0] = right[2]; *r[2][1] = up   [2]; *r[2][2] = at   [2];

  std::cout << std::endl;
  std::cout << " " << *r[0][0] << " " << *r[0][1] << " " << *r[0][2] << " " << *r[0][3] << std::endl;
  std::cout << " " << *r[1][0] << " " << *r[1][1] << " " << *r[1][2] << " " << *r[1][3] << std::endl;
  std::cout << " " << *r[2][0] << " " << *r[2][1] << " " << *r[2][2] << " " << *r[2][3] << std::endl;
  std::cout << " " << *r[3][0] << " " << *r[3][1] << " " << *r[3][2] << " " << *r[3][3] << std::endl;
  std::cout << std::endl;
  return r;
}

