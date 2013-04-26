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

#include <Interface/Modules/DataIO/WriteMatrixDialog.h>
#include <Core/Algorithms/DataIO/WriteMatrix.h>
#include <Dataflow/Network/ModuleStateInterface.h>  //TODO: extract into intermediate
#include <iostream>
#include <QFileDialog>

using namespace SCIRun::Gui;
using namespace SCIRun::Core::Algorithms::DataIO;
using namespace SCIRun::Dataflow::Networks;

WriteMatrixDialog::WriteMatrixDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent /* = 0 */)
  : ModuleDialogGeneric(state, parent)
{
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));
  fixSize();
  
  connect(saveFileButton_, SIGNAL(clicked()), this, SLOT(saveFile()));
  connect(fileNameLineEdit_, SIGNAL(editingFinished()), this, SLOT(pushFileNameToState()));
  connect(fileNameLineEdit_, SIGNAL(returnPressed()), this, SLOT(pushFileNameToState()));
  buttonBox->setVisible(false);
}

void WriteMatrixDialog::pull()
{
  fileNameLineEdit_->setText(QString::fromStdString(state_->getValue(WriteMatrixAlgorithm::Filename).getString()));
}

void WriteMatrixDialog::pushFileNameToState() 
{
  state_->setValue(WriteMatrixAlgorithm::Filename, fileNameLineEdit_->text().trimmed().toStdString());
}

void WriteMatrixDialog::saveFile()
{
  fileNameLineEdit_->setText(QFileDialog::getSaveFileName(this, "Save Matrix Text File", ".", "*.txt"));
  pushFileNameToState();
}