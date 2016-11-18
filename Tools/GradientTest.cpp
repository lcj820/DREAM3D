
#include <stdlib.h>

#include <fstream>
#include <iostream>
#include <string>

#include <SIMPLib/DataArrays/DataArray.hpp>

/**
 * @brief The ComputeGradient class computes the X and Y Gradient of a 2D array. This class mimics the
 * MATLAB function of the same name.
 */
class ComputeGradient
{
public:
  ComputeGradient(FloatArrayType::Pointer input, int width, int height)
  : m_Input(input)
  , m_Width(width)
  , m_Height(height)
  {
  }

  virtual ~ComputeGradient()
  {
  }


  FloatArrayType::Pointer getGradY() { return m_GradY; }

  FloatArrayType::Pointer getGradX() { return m_GradX; }

  void compute()
  {
    m_GradX = FloatArrayType::CreateArray(m_Input->getNumberOfTuples(), "X Gradient", true);
    m_GradX->initializeWithZeros();
    m_GradY = FloatArrayType::CreateArray(m_Input->getNumberOfTuples(), "Y Gradient", true);
    m_GradY->initializeWithZeros();

    float* f = m_Input->getPointer(0);

    size_t gIdx = 0;
    // Compute the X Gradient
    float* g = m_GradX->getPointer(0);
    std::vector<int> h(m_Height);
    int a = {1};
    std::generate(h.begin(), h.end(), [&a] { return a++; });
    // Take forward differences on top and bottom edges
    for(int col = 0; col < m_Width; col++)
    {
      int row = 0;
      gIdx = row * m_Width + col;
      g[gIdx] = (f[(row + 1) * m_Width + col] - f[row * m_Width + col]) / (h[row + 1] - h[row]);
      row = m_Height - 1;
      gIdx = row * m_Width + col;
      g[gIdx] = (f[(row)*m_Width + col] - f[(row - 1) * m_Width + col]) / (h[row] - h[row - 1]);
    }

    // Take centered differences on interior points
    for(int row = 1; row < m_Height - 1; row++)
    {
      for(int col = 0; col < m_Width; col++)
      {
        gIdx = row * m_Width + col;
        g[gIdx] = (f[(row + 1) * m_Width + col] - f[(row - 1) * m_Width + col]) / (h[row + 1] - h[row - 1]);
      }
    }

    // Compute the Y Gradient
    g = m_GradY->getPointer(0);
    h.resize(m_Width);
    a = {1};
    std::generate(h.begin(), h.end(), [&a] { return a++; });
    // Take forward differences on left and right edges
    for(int row = 0; row < m_Height; row++)
    {
      int col = 0;
      // g(:,1) = (f(:,2) - f(:,1))/(h(2)-h(1));
      gIdx = row * m_Width + col;
      g[gIdx] = (f[row * m_Width + col + 1] - f[row * m_Width + col]) / (h[col + 1] - h[col]);
      // g(:,n) = (f(:,n) - f(:,n-1))/(h(end)-h(end-1));
      col = m_Width - 1;
      gIdx = row * m_Width + col;
      g[gIdx] = (f[row * m_Width + col] - f[row * m_Width + col - 1]) / (h[col] - h[col - 1]);
    }

    // Take centered differences on interior points
    for(int row = 0; row < m_Height; row++)
    {
      for(int col = 1; col < m_Width - 1; col++)
      {
        gIdx = row * m_Width + col;
        g[gIdx] = (f[row * m_Width + col + 1] - f[row * m_Width + col - 1]) / (h[col + 1] - h[col - 1]);
      }
    }
  }

private:
  FloatArrayType::Pointer m_Input = FloatArrayType::NullPointer();
  int m_Width = -1;
  int m_Height = -1;
  FloatArrayType::Pointer m_GradX = FloatArrayType::NullPointer();
  FloatArrayType::Pointer m_GradY = FloatArrayType::NullPointer();
};


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
FloatArrayType::Pointer readFile(const std::string& filePath)
{
  FloatArrayType::Pointer values = FloatArrayType::NullPointer();
  std::vector<float> data;
  std::ifstream file(filePath);
  if(file.is_open())
  {
    float f;
    while(!file.eof())
    {
      file >> f;
      data.push_back(f);
    }
    file.close();
    values = FloatArrayType::FromStdVector(data, QString::fromStdString(filePath));
  }

  else
  {
    std::cout << "Unable to open file";
  }
  return values;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int main(int argc, char** argv)
{

  // Read The data from the file

  FloatArrayType::Pointer i_ob = readFile("/Users/mjackson/Desktop/Gradient/i_obj.csv");

  FloatArrayType::Pointer grad_x = readFile("/Users/mjackson/Desktop/Gradient/grad_x.csv");

  FloatArrayType::Pointer grad_y = readFile("/Users/mjackson/Desktop/Gradient/grad_y.csv");

  ComputeGradient grad(i_ob, 84, 95);
  grad.compute();

  FloatArrayType::Pointer gradY = grad.getGradY();
  FloatArrayType::Pointer gradX = grad.getGradX();

  size_t tupleCount = grad_x->getNumberOfTuples();
  for(size_t i = 0; i < tupleCount; i++)
  {
    if(grad_x->getValue(i) != gradX->getValue(i))
    {
      std::cout << "Non matching X value" << std::endl;
    }
    if(grad_y->getValue(i) != gradY->getValue(i))
    {
      std::cout << "Non matching Y Value" << std::endl;
    }

  }


  return EXIT_SUCCESS;
}
