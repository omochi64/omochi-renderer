#include <iostream>
#include <sstream>
#include <ctime>

#include "renderer/Renderer.h"
#include "renderer/Camera.h"
#include "renderer/LinearGammaToonMapper.h"
#include "scenes/CornellBoxScene.h"
#include "scenes/TestScene.h"
#include "tools/Vector.h"
#include "tools/PPM.h"
#include "viewer/WindowViewer.h"

using namespace std;
using namespace OmochiRenderer;

static const int supersampling = 4;

static const int startSample = 2;
static const int endSample = 1024;
static const int stepSample = 2;

class SavePPM_callback : public PathTracer::RenderingFinishCallback {
  int w,h;
public:
  SavePPM_callback(int width, int height):w(width),h(height){};
  void operator()(int samples, const Color *img) {
  	cerr << "save ppm file for sample " << samples << " ..." << endl;
    char name[1024];
    sprintf_s(name, 1024, "result_bvh_bunny_%04d_%dx%d.ppm", samples, supersampling, supersampling);
    clock_t begin,end;
    begin = clock();
    PPM::Save(name, img, w, h);
    end = clock();
    cerr << "saving time = " << (double)(end - begin)/CLOCKS_PER_SEC << endl;
  };
};

int main(int argc, char *argv[]) {

	int width = 640, height = 480;
	
  // set renderer and scene
  SavePPM_callback callback(width, height);
  Camera camera(width, height);
  PathTracer renderer(camera, startSample, endSample, stepSample, supersampling, &callback);
	TestScene scene;
  //CornellBoxScene scene;

  clock_t startTime;

  // set window viewer
  LinearGammaToonMapper mapper;
  WindowViewer viewer("OmochiRenderer", camera, renderer, mapper);
  viewer.StartViewerOnNewThread();
  viewer.SetCallbackFunctionWhenWindowClosed(std::function<void(void)>(
    [&startTime]{
      cerr << "total time = " << (1.0 / 60 * (clock() - startTime) / CLOCKS_PER_SEC) << " (min)." << endl;
      exit(0);
    }
  ));

  // start
  cerr << "begin rendering..." << endl;
  startTime = clock();
	renderer.RenderScene(scene);
  cerr << "total time = " << (1.0 / 60 * (clock() - startTime) / CLOCKS_PER_SEC) << " (min)." << endl;

	return 0;
}
