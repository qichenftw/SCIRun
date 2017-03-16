/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
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
/// @todo Documentation Modules/Visualization/ShowString.cc

#include <Modules/Visualization/ShowString.h>
#include <Modules/Visualization/TextBuilder.h>
#include <Core/Datatypes/String.h>
#include <Graphics/Datatypes/GeometryImpl.h>

using namespace SCIRun;
using namespace Modules::Visualization;
using namespace Core::Datatypes;
using namespace Dataflow::Networks;
using namespace Core::Algorithms;
using namespace Core::Algorithms::Visualization;
using namespace Core::Geometry;
using namespace Graphics::Datatypes;

ALGORITHM_PARAMETER_DEF(Visualization, TextRed);
ALGORITHM_PARAMETER_DEF(Visualization, TextGreen);
ALGORITHM_PARAMETER_DEF(Visualization, TextBlue);
ALGORITHM_PARAMETER_DEF(Visualization, TextAlpha);
ALGORITHM_PARAMETER_DEF(Visualization, FontName);
ALGORITHM_PARAMETER_DEF(Visualization, FontSize);
ALGORITHM_PARAMETER_DEF(Visualization, PositionType);
ALGORITHM_PARAMETER_DEF(Visualization, FixedHorizontal);
ALGORITHM_PARAMETER_DEF(Visualization, FixedVertical);
ALGORITHM_PARAMETER_DEF(Visualization, CoordinateHorizontal);
ALGORITHM_PARAMETER_DEF(Visualization, CoordinateVertical);

MODULE_INFO_DEF(ShowString, Visualization, SCIRun)

ShowString::ShowString() : GeometryGeneratingModule(staticInfo_), textBuilder_(boost::make_shared<TextBuilder>())
{
  INITIALIZE_PORT(String);
  INITIALIZE_PORT(RenderedString);
}

void ShowString::setStateDefaults()
{
  auto state = get_state();
  state->setValue(Parameters::TextRed, 1.0);
  state->setValue(Parameters::TextGreen, 1.0);
  state->setValue(Parameters::TextBlue, 1.0);
  state->setValue(Parameters::TextAlpha, 1.0);
  state->setValue(Parameters::FontSize, 16);

  state->setValue(Parameters::FontName, std::string("FreeSans.ttf"));
  state->setValue(Parameters::PositionType, std::string("Preset"));
  state->setValue(Parameters::FixedHorizontal, std::string("Left"));
  state->setValue(Parameters::FixedVertical, std::string("Top"));
  state->setValue(Parameters::CoordinateHorizontal, 0.5);
  state->setValue(Parameters::CoordinateVertical, 0.5);
}

void ShowString::execute()
{
  auto str = getRequiredInput(String);

  if (needToExecute())
  {
    auto geom = buildGeometryObject(str->value());
    sendOutput(RenderedString, geom);
  }
}

std::tuple<double, double> ShowString::getTextPosition() const
{
  auto state = get_state();
  auto positionChoice = state->getValue(Parameters::PositionType).toString();
  if ("Preset" == positionChoice)
  {
    return std::make_tuple(1.0, 1.0);
  }
  else if ("Coordinates" == positionChoice)
  {
    return std::make_tuple(1.0, 1.0);
  }
  else
  {
    throw "logical error";
  }
}

// TODO: clean up duplication here and in ShowColorMap
GeometryBaseHandle ShowString::buildGeometryObject(const std::string& text)
{
  std::vector<Vector> points;
  std::vector<ColorRGB> colors;
  std::vector<uint32_t> indices;
  auto numVBOElements = 0;

  // IBO/VBOs and sizes
  uint32_t iboSize = sizeof(uint32_t) * static_cast<uint32_t>(indices.size());
  uint32_t vboSize = sizeof(float) * 7 * static_cast<uint32_t>(points.size());

  std::shared_ptr<CPM_VAR_BUFFER_NS::VarBuffer> iboBufferSPtr(new CPM_VAR_BUFFER_NS::VarBuffer(vboSize));
  std::shared_ptr<CPM_VAR_BUFFER_NS::VarBuffer> vboBufferSPtr(new CPM_VAR_BUFFER_NS::VarBuffer(iboSize));

  CPM_VAR_BUFFER_NS::VarBuffer* iboBuffer = iboBufferSPtr.get();
  CPM_VAR_BUFFER_NS::VarBuffer* vboBuffer = vboBufferSPtr.get();

  for (auto a : indices) iboBuffer->write(a);

  for (size_t i = 0; i < points.size(); i++) 
  {
    vboBuffer->write(static_cast<float>(points[i].x()));
    vboBuffer->write(static_cast<float>(points[i].y()));
    vboBuffer->write(static_cast<float>(points[i].z()));
    vboBuffer->write(static_cast<float>(colors[i].r()));
    vboBuffer->write(static_cast<float>(colors[i].g()));
    vboBuffer->write(static_cast<float>(colors[i].b()));
    vboBuffer->write(static_cast<float>(1.f));
  }

  auto uniqueNodeID = get_id().id_ + "_showString_" + text;
  auto vboName = uniqueNodeID + "VBO";
  auto iboName = uniqueNodeID + "IBO";
  auto passName = uniqueNodeID + "Pass";

  // Construct VBO.
  std::string shader = "Shaders/ColorMapLegend";
  std::vector<SpireVBO::AttributeData> attribs;
  attribs.push_back(SpireVBO::AttributeData("aPos", 3 * sizeof(float)));
  attribs.push_back(SpireVBO::AttributeData("aColor", 4 * sizeof(float)));
  std::vector<SpireSubPass::Uniform> uniforms;

  auto position = getTextPosition();
  int xTrans = std::get<0>(position); 
  int yTrans = std::get<1>(position);

  uniforms.push_back(SpireSubPass::Uniform("uXTranslate", static_cast<float>(xTrans)));
  uniforms.push_back(SpireSubPass::Uniform("uYTranslate", static_cast<float>(yTrans)));
  
  SpireVBO geomVBO(vboName, attribs, vboBufferSPtr, numVBOElements, BBox(), true);
  SpireIBO geomIBO(iboName, SpireIBO::PRIMITIVE::TRIANGLES, sizeof(uint32_t), iboBufferSPtr);

  RenderState renState;
  renState.set(RenderState::IS_ON, true);
  renState.set(RenderState::HAS_DATA, true);

  SpireText spiretext;

  SpireSubPass pass(passName, vboName, iboName, shader,
    ColorScheme::COLOR_MAP, renState, RenderType::RENDER_VBO_IBO, geomVBO, geomIBO, spiretext);

  // Add all uniforms generated above to the pass.
  for (const auto& uniform : uniforms) { pass.addUniform(uniform); }

  GeometryHandle geom(new GeometryObjectSpire(*this, "ShowString", false));

  geom->mIBOs.push_back(geomIBO);
  geom->mVBOs.push_back(geomVBO);
  geom->mPasses.push_back(pass);

  auto state = get_state();
  auto fontSize = state->getValue(Parameters::FontSize).toInt();
  auto fontName = state->getValue(Parameters::FontName).toString() + ".ttf";

  if (textBuilder_ && textBuilder_->isReady() && textBuilder_->getFontName() != fontName)
  {
    textBuilder_.reset(new TextBuilder);
  }

  if (!textBuilder_->initialize(fontSize, fontName))
    return geom;

  if (textBuilder_->getFaceSize() != fontSize)
    textBuilder_->setFaceSize(fontSize);

  {
    auto r = state->getValue(Parameters::TextRed).toDouble();
    auto g = state->getValue(Parameters::TextGreen).toDouble();
    auto b = state->getValue(Parameters::TextBlue).toDouble();
    auto a = state->getValue(Parameters::TextAlpha).toDouble();

    textBuilder_->setColor(r, g, b, a);
  }

  Vector trans(xTrans, yTrans, 0.0); 
  textBuilder_->printString(text, trans, Vector(), text, *geom);

  return geom;
}
