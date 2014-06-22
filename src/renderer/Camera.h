#pragma once

#include "tools/Vector.h"
#include "Ray.h"

#include "Aperture.h"

namespace OmochiRenderer {

  class Camera {
  public:
    Camera(size_t screenWidth, size_t screenHeight, const Vector3 &cameraPos = Vector3(50.0, 52.0, 220.0),
      const Vector3 &cameraDir = Vector3(0.0, -0.04, -1.0),
      const Vector3 &cameraUp = Vector3(0.0, 1.0, 0.0),
      const double screenHeightIn3Dcoordinate = 30.0, const double distanceToScreen = 40,
      const double focalDistance = 200)
      : m_width(screenWidth)
      , m_height(screenHeight)
      , m_camPos(cameraPos)
      , m_camDir(cameraDir)
      , m_camUp(cameraUp)
      , m_nearScreenHeight(screenHeightIn3Dcoordinate)
      , m_distToScreen(distanceToScreen)
      , m_focalDistance(focalDistance)
      , m_aperture()
    {
      m_camDir.normalize();
      m_camUp.normalize();

      double nearScreenHeight = m_nearScreenHeight;
      double nearScreenWidth = m_nearScreenHeight * m_width / m_height;

      m_screenCenter = m_camPos + m_camDir * m_distToScreen;

      // �X�N���[��x�����𒣂�x�N�g��
      m_screenXaxis = m_camDir.cross(m_camUp);
      m_screenXaxis.normalize(); m_screenXaxis *= nearScreenWidth;

      // �X�N���[��y�����𒣂�x�N�g��
      m_screenYaxis = m_screenXaxis.cross(m_camDir);
      m_screenYaxis.normalize(); m_screenYaxis *= nearScreenHeight;

    }

    void SetAperture(std::shared_ptr<Aperture> aperture)
    {
      m_aperture = aperture;
    }

    inline const size_t GetScreenWidth() const {return m_width;}
    inline const size_t GetScreenHeight() const {return m_height;}
    inline const Vector3 &GetCameraPosition() const {return m_camPos;}

    inline const Vector3 &GetScreenCenterPosition() const {
      return m_screenCenter;
    }

    inline const Vector3 &GetScreenAxisXvector() const {
      return m_screenXaxis;
    }
    inline const Vector3 &GetScreenAxisYvector() const {
      return m_screenYaxis;
    }

    Ray SampleRayForPixel(double x, double y, const Random &rnd) const {

      // ���ʂ� ray (center_ray) �̕���
      Vector3 target_position = m_screenCenter +
        m_screenXaxis * (x / m_width - 0.5) +
        m_screenYaxis * (y / m_height - 0.5);
      Vector3 target_dir = target_position - GetCameraPosition();
      target_dir.normalize();

      Ray ray(GetCameraPosition(), target_dir);

      if (m_aperture)
      {
        double diffx, diffy;
        m_aperture->SampleOnePoint(diffx, diffy, rnd);

        // center_ray �� focal plane �ɏՓ˂��� t ( ray = b_ + t*v ) �ƏՓ˂���ꏊ�����߂�
        double focusTimeForNormalRay = m_focalDistance / fabs(target_dir.z);
        Vector3 focusPos(ray(focusTimeForNormalRay));

        // �T���v�����O���ꂽ�����Y��̈ʒu
        ray.orig = GetCameraPosition() + Vector3(diffx, diffy, 0);

        // ���ۂ̕��������߂�
        ray.dir = (focusPos - ray.orig).normalize();
      }

      return ray;
    }

  private:
	  size_t m_width, m_height;
	  //int m_min_samples,m_max_samples,m_step_samples;
	  //int m_supersamples;

	  Vector3 m_camPos;
	  Vector3 m_camDir;
	  Vector3 m_camUp;
	  double m_nearScreenHeight;
	  double m_distToScreen;

    Vector3 m_screenXaxis, m_screenYaxis;
    Vector3 m_screenCenter;

    // lens
    double m_focalDistance;

    // aperture
    std::shared_ptr<Aperture> m_aperture;

  };

}
