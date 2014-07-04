#pragma once

#include "tools/Vector.h"
#include "tools/Random.h"
#include "tools/Constant.h"

namespace OmochiRenderer {

  // �i��̃C���^�[�t�F�[�X
  class Aperture {
  public:
    virtual void SampleOnePoint(double &sampledX, double &sampledY, const Random &rnd) const = 0;
  };

  // �~�`�̍i��
  class CircleAperture : public Aperture {
  public:
    explicit CircleAperture(double radius = 0.0)
      : m_radius(radius)
    {
    }

    virtual void SampleOnePoint(double &sampledX, double &sampledY, const Random &rnd) const {
      // 1�_���T���v�����O
      // F(r, ��) = ��/2PI * r^2/R^2

      // F(��) = ��/2PI
      double theta = 2 * PI * rnd.nextDouble();

      // F(r) = r^2/R^2
      double r = m_radius * sqrt(rnd.nextDouble());

      sampledX = r * cos(theta);
      sampledY = r * sin(theta);
    }

  private:
    double m_radius;
  };

}
