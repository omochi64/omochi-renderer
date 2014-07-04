#include "stdafx.h"

#include "TestScene.h"
#include "renderer/Sphere.h"
#include "renderer/Polygon.h"
#include "renderer/Model.h"
#include "renderer/BVH.h"
#include "renderer/SphereLight.h"
#include "tools/ImageHandler.h"

namespace OmochiRenderer {

TestScene::TestScene()
{
  auto tex = ImageHandler::GetInstance().LoadFromFile("input_data/chino.jpg", true);

  //AddObject(new Sphere(1e5, Vector3(1e5 + 1, 40.8, 81.6), Material(Material::REFLECTION_TYPE_LAMBERT, Color(), Color(0.75, 0.25, 0.25), 0.0, tex)), true, false);  // ��
  //AddFloorYZ_xUp(200, 200, Vector3(1, 40.8, 81.6), Material(Material::REFLECTION_TYPE_LAMBERT, Color(), Color(0.75, 0.25, 0.25), 0.0)); // ��
  ////AddObject(new Sphere(1e5, Vector3(-1e5 + 99, 40.8, 81.6), Material(Material::REFLECTION_TYPE_LAMBERT, Color(), Color(0.25, 0.25, 0.75), tex)), true, false);  // �E
  //AddFloorYZ_xDown(200, 200, Vector3(99, 40.8, 81.6), Material(Material::REFLECTION_TYPE_LAMBERT, Color(), Color(0.25, 0.25, 0.75), 0.0)); // �E
  ////AddObject(new Sphere(1e5, Vector3(50, 40.8, 1e5), Material(Material::REFLECTION_TYPE_LAMBERT, Color(), Color(0.75, 0.75, 0.75))), true, false);  // ��
  //AddFloorXY_zUp(150, 150/1.77777777777, Vector3(50, 40.8, 0), Material(Material::REFLECTION_TYPE_LAMBERT, Color(), Color(1, 1, 1), 0.0, tex));  // ��
  ////AddObject(new Sphere(1e5, Vector3(50, 40.8, -1e5 + 250), Material(Material::REFLECTION_TYPE_LAMBERT, Color(), Color())), true, false);                  // ��O
  //AddFloorXY_zDown(150, 150 / 1.77777777777, Vector3(50, 40.8, 250), Material(Material::REFLECTION_TYPE_LAMBERT, Color(), Color(0.75, 0.75, 0.75), 0.0, tex));  // ��O
  ////AddObject(new Sphere(1e5, Vector3(50, 1e5, 81.6), Material(Material::REFLECTION_TYPE_LAMBERT, Color(), Color(0.75, 0.75, 0.75))), true, false);  // ��
  //AddFloorXZ_yUp(200, 200, Vector3(50, 0, 81.6), Material(Material::REFLECTION_TYPE_LAMBERT, Color(), Color(0.75, 0.75, 0.75)));
  ////AddObject(new Sphere(1e5, Vector3(50, -1e5 + 81.6, 81.6), Material(Material::REFLECTION_TYPE_LAMBERT, Color(), Color(0.75, 0.75, 0.75))), true, false);  // ��
  //AddFloorXZ_yDown(200, 200, Vector3(50, 81.6, 81.6), Material(Material::REFLECTION_TYPE_LAMBERT, Color(), Color(0.75, 0.75, 0.75)));

 
  Model *cube = new Model;
  if (!cube->ReadFromObj("input_data/ocean.obj", true)) {
    std::cerr << "failed to load cube.obj!!!" << std::endl;
    getchar();
    exit(-1);
  }
  cube->SetTransform(Vector3(50, 30, 50), Vector3(10, 10, 10), Matrix::RotateAroundVector(Vector3(0, 1, 0), 45.0 / 180 * PI));
  //cube->SetTransform(Vector3(50, 10, 50), Vector3(6, 6, 6), Matrix::RotateAroundVector(Vector3(0, 1, 0), 30.0 / 180 * PI));
  //cube->SetTransform(Vector3(0, 0, -30), Vector3(5, 5, 5), Matrix::RotateAroundVector(Vector3(0, 1, 0), 90.0 / 180 * PI));
  AddModel(cube);
  
  //AddObject(new Sphere(10,Vector3(),           Material(Material::REFLECTION_TYPE_LAMBERT,    Color(), Color(0.25, 0.75, 0.25))));    // �΋�
  //AddObject(new Sphere(16.5,Vector3(27, 16.5, 47),       Material(Material::REFLECTION_TYPE_SPECULAR,   Color(), Color(0.99, 0.99, 0.99))));   // ��
  //AddObject(new Sphere(16.5,Vector3(77, 16.5, 78),       Material(Material::REFLECTION_TYPE_REFRACTION, Color(), Color(0.99, 0.99, 0.99), REFRACTIVE_INDEX_OBJECT))); // �K���X


  SphereLight *sphereLight = new SphereLight(7.5, Vector3(50.0, 72.5, 81.6), Material(Material::REFLECTION_TYPE_LAMBERT, Color(26, 26, 26), Color()));
  //SphereLight *sphereLight = new SphereLight(15, Vector3(50.0, 90, 81.6), Material(Material::REFLECTION_TYPE_LAMBERT, Color(36, 36, 36), Color()));
  AddObject(sphereLight, true, false);    // �Ɩ�

  //ConstructBVH();
  ConstructQBVH();
}

}
