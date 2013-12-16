#include <iostream>
#include <ctime>
#include "Color.h"
#include "scenes/Scene.h"
#include "Renderer.h"
#include "Ray.h"
#include "tools/Random.h"
#include "IBL.h"

#include <sstream>

using namespace std;

namespace OmochiRenderer {

PathTracer::PathTracer(const Camera &camera, int samples, int supersamples)
  : m_camera(camera)
{
  init(camera, samples, samples, 1, supersamples, NULL);
}

PathTracer::PathTracer(const Camera &camera, int min_samples, int max_samples, int steps, int supersamples, RenderingFinishCallback *callback)
  : m_camera(camera)
{
  init(camera, min_samples, max_samples, steps, supersamples, callback);
}

void PathTracer::init(const Camera &camera, int min_samples, int max_samples, int steps, int supersamples, RenderingFinishCallback *callback)
{
  SetCamera(camera);
	m_min_samples = (min_samples);
  m_max_samples = max_samples;
  m_step_samples = steps;
	m_supersamples = (supersamples);
  m_previous_samples = 0;
  m_renderFinishCallback = callback;

  m_checkIntersectionCount = 0;
  m_result = new Color[m_camera.GetScreenHeight()*m_camera.GetScreenWidth()];
}

PathTracer::~PathTracer()
{
	delete [] m_result;
}

void PathTracer::RenderScene(const Scene &scene) {

  // �X�N���[�����S
  const Vector3 screen_center = m_camera.GetScreenCenterPosition();

  m_omittedRayCount = 0;
  m_hitToLightCount = 0;
  m_previous_samples = 0;
  for (int samples=m_min_samples; samples<=m_max_samples; samples+=m_step_samples) {
    clock_t t1, t2;
    t1 = clock();
    m_checkIntersectionCount = 0;
    ScanPixelsAndCastRays(scene, m_previous_samples, samples);
    t2 = clock();
    m_previous_samples = samples;
    cerr << "samples = " << samples << " rendering finished." << endl;
    double pastsec = 1.0*(t2-t1)/CLOCKS_PER_SEC;
    cerr << "rendering time = " << (1.0/60)*pastsec << " min." << endl;
    cerr << "speed = " << m_checkIntersectionCount/pastsec*(samples-m_previous_samples) << " rays (intersection check)/sec" << endl;
    if (m_renderFinishCallback) {
      (*m_renderFinishCallback)(samples, m_result, pastsec/60.0);
    }
  }
}

void PathTracer::ScanPixelsAndCastRays(const Scene &scene, int previous_samples, int next_samples) {
  m_processed_y_counts = 0;

  const size_t height = m_camera.GetScreenHeight();
  const size_t width = m_camera.GetScreenWidth();
  const Vector3 &screen_x(m_camera.GetScreenAxisXvector());
  const Vector3 &screen_y(m_camera.GetScreenAxisYvector());
  const Vector3 &screen_center(m_camera.GetScreenCenterPosition());

  // trace all pixels
  const double averaging_factor = next_samples * m_supersamples * m_supersamples;
#pragma omp parallel for schedule(dynamic, 1)
  for (int y=0; y<height; y++) {
    Random rnd(y+1+previous_samples*height);
    for (int x=0; x<width; x++) {
      const int index = x + (height - y - 1)*width;

      Color accumulated_radiance;
       
      // super-sampling
      for (int sy=0; sy<m_supersamples; sy++) for (int sx=0; sx < m_supersamples; sx++) {
        // (x,y)�s�N�Z�����ł̈ʒu: [0,1]
        const double rx = (2.0*sx + 1.0)/(2*m_supersamples);
        const double ry = (2.0*sy + 1.0)/(2*m_supersamples);

        Vector3 target_position = screen_center +
          screen_x * ((x+rx)/width - 0.5) +
          screen_y * ((y+ry)/height - 0.5);
        Vector3 target_dir = target_position - m_camera.GetCameraPosition();
        target_dir.normalize();

        // (m_samples)��T���v�����O����
        for (int s=previous_samples+1; s<=next_samples; s++) {
          accumulated_radiance += Radiance(scene, Ray(m_camera.GetCameraPosition(), target_dir), rnd, 0);
          m_omittedRayCount++;
        }
      }
      // img_n+c(x) = n/(n+c)*img_n(x) + 1/(n+c)*sum_{n+1}^{n+c}rad_i(x)/supersamples^2
      m_result[index] = m_result[index] * (static_cast<double>(previous_samples) / next_samples) + accumulated_radiance / averaging_factor;
    }
    m_processed_y_counts++;
    //cerr << "y = " << y << ": " << static_cast<double>(m_processed_y_counts)/height*100 << "% finished" << endl;

  }
}

const static int MinDepth = 5;
const static int MaxDepth = 64;

Color PathTracer::Radiance(const Scene &scene, const Ray &ray, Random &rnd, const int depth) {
  Scene::IntersectionInformation intersect;

  m_checkIntersectionCount++;
  bool intersected = scene.CheckIntersection(ray, intersect);

  Vector3 normal;

  if (intersected) {
    normal = intersect.hit.normal.dot(ray.dir) < 0.0 ? intersect.hit.normal : intersect.hit.normal * -1.0;
  }

  Color income;// = DirectRadiance(scene, ray, rnd, depth, intersected, intersect, normal);
  income += Radiance_internal(scene, ray, rnd, depth, intersected, intersect, normal);


  return income;
}

/*
Color PathTracer::DirectRadiance(const Scene &scene, const Ray &ray, Random &rnd, const int depth, const bool intersected, Scene::IntersectionInformation &intersect, const Vector3 &normal) {
  if (!intersected) {
    if (scene.GetIBL()) {
      return scene.GetIBL()->Sample(ray);    // ���m�ɔw�i�Ƃ̏Փˈʒu���v�Z����
      // return scene.GetIBL()->Sample(ray.dir); // ���C�����_����n�܂��Ă���Ƃ݂Ȃ��Čv�Z����
    } else {
      return scene.Background();
    }
  }

  Color income;

  if (intersect.object->material.emission.lengthSq() == 0) {
    switch (intersect.object->material.reflection_type) {
    case Material::REFLECTION_TYPE_LAMBERT:
      // ���C�g����T���v�����O���s��
      // IBL ���܂߂�
      //income = DirectRadiance_Lambert(scene, ray, rnd, depth, intersected, intersect, normal);
      break;

    case Material::REFLECTION_TYPE_SPECULAR:
    case Material::REFLECTION_TYPE_REFRACTION:
      // �Ԑڌ��]���݂̂ŗǂ�
      break;
    }
  } else {
    m_hitToLightCount++;
  }

  if (depth == 0)
    income += intersect.object->material.emission;

  return income;
}
*/

Color PathTracer::DirectRadiance_Lambert(const Scene &scene, const Ray &ray, Random &rnd, const int depth, const bool intersected, Scene::IntersectionInformation &intersect, const Vector3 &normal) {
  assert(intersected);

  const vector<LightBase *> &lights = scene.GetLights();
  if (lights.size() == 0) return scene.Background();

  // pick a random light according to light power
  double totalPower = 0.0;
  vector<double> eachLightProbability, accumulatedProbability;
  for (size_t i = 0; i < lights.size(); i++) {
    double power = lights[i]->TotalPower();
    eachLightProbability.push_back(power);
    accumulatedProbability.push_back(totalPower + power);
    totalPower += power;
  }

  for (size_t i = 0; i < lights.size(); i++) {
    eachLightProbability[i] /= totalPower;
    accumulatedProbability[i] /= totalPower;
  }

  static const int NumberOfLightSamples = 1;
  Color income;

  for (int lightCount = 0; lightCount < NumberOfLightSamples; lightCount++) {

    double next = rnd.nextDouble();
    int index = 0;
    for (size_t i = 0; i < lights.size(); i++) {
      if (next <= accumulatedProbability[i]) {
        index = i; break;
      }
    }

    const LightBase *selectedLight = lights[index];

    // pick a one point
    Vector3 point, light_normal; double pdf = 0.0;
    selectedLight->SampleOnePoint(point, light_normal, pdf, rnd);

    Vector3 dir((point - intersect.hit.position)); dir.normalize();
    double cos_shita = dir.dot(normal);
    double light_cos_shita = -dir.dot(light_normal);
    if (cos_shita < 0 || light_cos_shita < 0) {
      // cannot reach to the light
      continue;
    }
  
    // check visibility
    Scene::IntersectionInformation hit;
    if (scene.CheckIntersection(Ray(intersect.hit.position, dir), hit)) {
      if (dynamic_cast<LightBase *>(hit.object) == selectedLight) {
        // visible
        // BRDF = color/PI
        double G = cos_shita * light_cos_shita / (hit.hit.distance * hit.hit.distance);
        Vector3 reflect_rate(intersect.object->material.color / PI * G / (pdf * eachLightProbability[index]));
        income.x += reflect_rate.x * hit.object->material.emission.x;
        income.y += reflect_rate.y * hit.object->material.emission.y;
        income.z += reflect_rate.z * hit.object->material.emission.z;
        // direct_illum = 1/N*��L_e*BRDF*G*V/pdf(light)
        m_hitToLightCount++;
      }
    }
    m_omittedRayCount++;
  }

  return income / NumberOfLightSamples;
}

Color PathTracer::Radiance_internal(const Scene &scene, const Ray &ray, Random &rnd, const int depth, const bool intersected, Scene::IntersectionInformation &intersect, const Vector3 &normal) {
  if (!intersected) {
    if (scene.GetIBL()) {
      return scene.GetIBL()->Sample(ray);    // ���m�ɔw�i�Ƃ̏Փˈʒu���v�Z����
      // return scene.GetIBL()->Sample(ray.dir); // ���C�����_����n�܂��Ă���Ƃ݂Ȃ��Čv�Z����
    } else {
      return scene.Background();
    }
  }

  Color income;

  // ���� if ����L���ɂ���ƁA���ڌ��̂ݍl������悤�ɂȂ�
  //if (depth >= 1) {
  //  return intersect.object->material.emission;
  //}

  double russian_roulette_probability = std::max(intersect.object->material.color.x, std::max(intersect.object->material.color.y, intersect.object->material.color.z)); // �K��
  if (depth > MaxDepth) {
    russian_roulette_probability *= pow(0.5, depth - MaxDepth);
  }
  if (depth > MinDepth) {
    if (rnd.nextDouble() >= russian_roulette_probability) {
      // �e radiance ���v�Z����Ƃ��ɒ��ڌ����v�Z���Ă���̂ŁA
      // �ueye ���璼�� light �� hit �����ꍇ�v�̂݁Aemission �� income �ɉ�����
      if (depth == 0 || !m_performNextEventEstimation) {
        if (intersect.object->material.emission.lengthSq() != 0) {
          m_hitToLightCount++;
        }
        return intersect.object->material.emission;
      }
      return scene.Background();
    }
  } else {
    russian_roulette_probability = 1.0; // no roulette
  }

  switch (intersect.object->material.reflection_type) {
    case Material::REFLECTION_TYPE_LAMBERT:
      income = Radiance_Lambert(scene, ray, rnd, depth, intersect, normal, russian_roulette_probability);
      break;
    case Material::REFLECTION_TYPE_SPECULAR:
      income = Radiance_Specular(scene, ray, rnd, depth, intersect, normal, russian_roulette_probability);
      break;
    case Material::REFLECTION_TYPE_REFRACTION:
      income = Radiance_Refraction(scene, ray, rnd, depth, intersect, normal, russian_roulette_probability);
      break;
  }

  // �e radiance ���v�Z����Ƃ��ɒ��ڌ����v�Z���Ă���̂ŁA
  // �ueye ���璼�� light �� hit �����ꍇ�v�̂݁Aemission �� income �ɉ�����
  if (depth == 0 || !m_performNextEventEstimation) {
    // --> eye ���璼�� light �� hit ���A light ���g�����˗������ꍇ�ɂ������L���ɂȂ�
    income += intersect.object->material.emission;
    if (intersect.object->material.emission.lengthSq() != 0) {
      m_hitToLightCount++;
    }
  }

  return income;
}


Color PathTracer::Radiance_Lambert(const Scene &scene, const Ray &ray, Random &rnd, const int depth, Scene::IntersectionInformation &intersect, const Vector3 &normal, double russian_roulette_prob) {

  Color direct;

  // ���ڌ���]������
  if (m_performNextEventEstimation && intersect.object->material.emission.lengthSq() == 0) {
    direct = DirectRadiance_Lambert(scene, ray, rnd, depth, true, intersect, normal);
  }

  Vector3 w,u,v;

  w = normal;
  if (fabs(normal.x) > EPS) {
   u = Vector3(0,1,0).cross(w);
  } else {
   u = Vector3(1,0,0).cross(w);
  }
  v = w.cross(u);
  double u1 = rnd.nextDouble(); double u2 = rnd.nextDouble();
  // pdf is 1/PI
  //double r1 = PI*u1; // ��
  //double r2 = 1-u2; // cos��
  //double r3 = sqrt(1-r2*r2); // sin��
  //double pdf = 1/PI;

  // pdf is cos��/PI
  double r1 = 2*PI*u1;
  double r2 = sqrt(u2); // cos��
  double r3 = sqrt(1-u2); // sin��
  double pdf = r2/PI;

  //const double r1 = 2 * PI * rnd.nextDouble();
  //const double r2 = rnd.nextDouble(), r2s = sqrt(r2);
  //Vector3 dir = u * cos(r1) * r2s + v * sin(r1) * r2s + w * sqrt(1.0-r2);
  //dir.normalize();
  //weight = intersect.object->color / russian_roulette_probability;

  Vector3 dir = u*r3*cos(r1) + v*r3*sin(r1) + w*r2;
  dir.normalize();

  //Color weight = intersect.object->color / PI * r2 / pdf / russian_roulette_prob;
  Color weight = intersect.object->material.color;
  Color income = Radiance(scene, Ray(intersect.hit.position, dir), rnd, depth+1);
  // direct �͂��łɔ��˗�����Z�ς݂Ȃ̂ŁAweight���|����K�v�͂Ȃ�
  return ( Vector3(weight.x*income.x, weight.y*income.y, weight.z*income.z) + direct ) / russian_roulette_prob;
}

// ���ʔ���
Color PathTracer::Radiance_Specular(const Scene &scene, const Ray &ray, Random &rnd, const int depth, Scene::IntersectionInformation &intersect, const Vector3 &normal, double russian_roulette_prob) {
  Vector3 reflected_dir(ray.dir - normal*2*ray.dir.dot(normal));
  reflected_dir.normalize();

  // �Ԑڌ��̕]��
  Ray newray(intersect.hit.position, reflected_dir);
  Color income = Radiance(scene, newray, rnd, depth+1);

  // ���ڌ��̕]��
  Scene::IntersectionInformation newhit;
  if (m_performNextEventEstimation && scene.CheckIntersection(newray, newhit)) {
    income += newhit.object->material.emission;
  }

  Color weight = intersect.object->material.color / russian_roulette_prob;
  return Vector3(weight.x*income.x, weight.y*income.y, weight.z*income.z);

}

// ���ܖ�
Color PathTracer::Radiance_Refraction(const Scene &scene, const Ray &ray, Random &rnd, const int depth, Scene::IntersectionInformation &intersect, const Vector3 &normal, double russian_roulette_prob) {
  bool into = intersect.hit.normal.dot(normal) > 0.0;

  Vector3 reflect_dir = ray.dir - normal*2*ray.dir.dot(normal);
  reflect_dir.normalize();
  double n_vacuum = REFRACTIVE_INDEX_VACUUM;
  double n_obj = intersect.object->material.refraction_rate;
  double n_ratio = into ? n_vacuum/n_obj : n_obj/n_vacuum;

  double dot = ray.dir.dot(normal);
  double cos2t = 1-n_ratio*n_ratio*(1-dot*dot);

  // ���˕����̒��ڌ��̕]��
  Color reflect_direct;
  Ray reflect_ray(intersect.hit.position, reflect_dir);
  Scene::IntersectionInformation reflected_hit;
  if (m_performNextEventEstimation && scene.CheckIntersection(reflect_ray, reflected_hit)) {
    reflect_direct = reflected_hit.object->material.emission;
  }

  if (cos2t < 0) {
    // �S����
    Color income = reflect_direct + Radiance(scene, Ray(intersect.hit.position, reflect_dir), rnd, depth+1);
    Color weight = intersect.object->material.color / russian_roulette_prob;
    return Vector3(weight.x*income.x, weight.y*income.y, weight.z*income.z);
  }

  // ���ܕ���
  Vector3 refract_dir( ray.dir*n_ratio - intersect.hit.normal * (into ? 1.0 : -1.0) * (dot*n_ratio + sqrt(cos2t)) );
  refract_dir.normalize();
  const Ray refract_ray(intersect.hit.position, refract_dir);
  // ���ܕ����̒��ڌ��̕]��
  Color refract_direct;
  if (m_performNextEventEstimation && scene.CheckIntersection(refract_ray, reflected_hit)) {
    refract_direct = reflected_hit.object->material.emission;
  }

  // Fresnel �̎�
  double F0 = (n_obj-n_vacuum)*(n_obj-n_vacuum)/((n_obj+n_vacuum)*(n_obj+n_vacuum));
  double c = 1 - ( into ? -dot : -refract_dir.dot(normal) );  // 1-cos��
  double Fr = F0 + (1-F0)*pow(c, 5.0);    // Fresnel (���˂̊���)
  double n_ratio2 = n_ratio*n_ratio;  // ���ܑO��ł̕��ˋP�x�̕ω���
  double Tr = (1-Fr)*n_ratio2;        // ���ܒ��と���O�̊���

  Color income, weight;

  if (depth > 2) {
    // ���� or ���܂̂ݒǐ�
    const double reflect_prob = 0.1 + 0.8 * Fr;
    if (rnd.nextDouble() < reflect_prob) {
      // ����
      income = (reflect_direct + Radiance(scene, Ray(intersect.hit.position, reflect_dir), rnd, depth+1)) * Fr;
      weight = intersect.object->material.color / (russian_roulette_prob * reflect_prob);
    } else {
      // ����
      income = (refract_direct + Radiance(scene, refract_ray, rnd, depth+1)) * Tr;
      weight = intersect.object->material.color / (russian_roulette_prob * (1-reflect_prob));
    }
  } else {
    // ���˂Ƌ��ܗ����ǐ�
    m_omittedRayCount++;
    income =
      (reflect_direct + Radiance(scene, Ray(intersect.hit.position, reflect_dir), rnd, depth+1)) * Fr +
      (refract_direct + Radiance(scene, refract_ray, rnd, depth + 1)) *Tr;
    weight = intersect.object->material.color / russian_roulette_prob;
  }

  return /*intersect.object->material.emission +*/ Vector3(weight.x*income.x, weight.y*income.y, weight.z*income.z);
}

std::string PathTracer::GetCurrentRenderingInfo() const {

  stringstream ss;
  ss << "(width, height) = (" << m_camera.GetScreenWidth() << ", " << m_camera.GetScreenHeight() << ")" << endl;
  ss << "previous samples / pixel = " << m_previous_samples << "x(" << m_supersamples << "x" << m_supersamples << ")" << endl;
  ss << "current rendering samples / pixel = " << (m_previous_samples+m_step_samples) << "x(" << m_supersamples << "x" << m_supersamples << ")" << endl;
  ss << "hit to light / omitted ray = " << m_hitToLightCount << " / " << m_omittedRayCount;
  if (m_omittedRayCount != 0) {
    ss << " = " << static_cast<double>(m_hitToLightCount*100.0) / m_omittedRayCount << "%";
  }
  ss << endl;
  ss << m_processed_y_counts*100.0/m_camera.GetScreenHeight() << "% finished." << endl;

  return ss.str();
}

}