//
// example7.cpp
// object motion w/Bezier interp with DeCasteljau's technique (cubic)
//

#include "ssg.h"
#include "Camera.h"
using namespace glm;

#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
using namespace std;

ModelNode *root;
Primitive *prim;
Camera camera;
int width, height;
vector<vec3> controlPoints;

float getNow() {
  return static_cast<double>(glutGet(GLUT_ELAPSED_TIME)) / 1000.0 ;
}

vec3 lerp (vec3 start, vec3 end, float u )
{
  return vec3 ( start*(1-u) + end*u );
}

vec3 deCasteljau2 ( std::vector<glm::vec3> &controlPoints, float u )
{
  //  std::cout << "u=" << u << " ";
  return lerp ( lerp (controlPoints[0], controlPoints[1], u),
		lerp (controlPoints[1], controlPoints[2], u),
		u);
}

vec3 deCasteljau3 ( vector<vec3> &controlPoints, float u )
{
  vector<vec3> interpCPs;
  interpCPs.push_back( lerp ( controlPoints[0], controlPoints[1], u ) );
  interpCPs.push_back( lerp ( controlPoints[1], controlPoints[2], u ) );
  interpCPs.push_back( lerp ( controlPoints[2], controlPoints[3], u ) );
  return deCasteljau2 ( interpCPs, u );
}

void display ()
{
  glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  static float lastFrame = getNow();
  float now = getNow();

  root->update(now-lastFrame);

  Instance *iroot = dynamic_cast<Instance*>(root);
  Instance *mover = dynamic_cast<Instance*> ( iroot->getChild(1) ); 

  // interpolate along a quadratic Bezier curve
  float motionStartTime = 5.0;
  float motionEndTime = motionStartTime+4.0;

  // move the mover

  float u = ((now - motionStartTime) / (motionEndTime - motionStartTime) );
  if ( u < 0.0 )
    mover->setMatrix ( translate(mat4(), controlPoints[0]) );
  else if ( u > 1.0 )
    mover->setMatrix ( translate(mat4(), controlPoints[3]) );
  else 
    mover->setMatrix ( translate(mat4(), deCasteljau3 (controlPoints, u ) ) );

  // add a marker to the scene at each position
  Instance *marker = new Instance();
  marker->addChild ( new Marker() );
  marker->setMatrix ( mover->getMatrix() );
  iroot->addChild ( marker );

  camera.draw(root);

  glutSwapBuffers();
  lastFrame = now;

  //  std::cout << now << std::endl;
}

void timer ( int delay )
{
  glutTimerFunc( delay, timer, delay );
  glutPostRedisplay();
}

void reshape (int w, int h)
{
  width = w;
  height = h;
  camera.setupPerspective ( w, h );
}


void keyboard (unsigned char key, int x, int y)
{
  switch (key) {

  case 27: /* ESC */
    exit(0);
    break;
  default:
    break;
  }
}

void init (int argc, char **argv)
{
  
  controlPoints.push_back ( vec3(0,0,0) );
  controlPoints.push_back ( vec3(3,0,0) );
  controlPoints.push_back ( vec3(-2,1,-3) );
  controlPoints.push_back ( vec3(2,2,-6) );

  //  create a primitive.  if supplied on command line, read a .obj wavefront file
  ObjFilePrimitive *background, *movingObject;
  if ( argc == 3 ) {
    background = new ObjFilePrimitive ( argv[1] );
    movingObject = new ObjFilePrimitive ( argv[2] );
  } else {
    cout << "usage: " << argv[0] << " backgroundObjFile moverObjfile\n";
    exit(1);
  }

  // create the graph
  // the second child of the root must be an instance: it will have its matrix changed
  Instance *scene = new Instance();
  scene->setMatrix ( mat4() );
  scene->addChild ( background );
  Instance *mover = new Instance();
  mover->addChild ( movingObject );
  scene->addChild ( mover );



  // enable camera trackball
  camera.enableTrackball (true);

  // the lights are global for all objects in the scene
  RenderingEnvironment::getInstance().lightPosition = vec4 ( 4,10,5,1 );
  RenderingEnvironment::getInstance().lightColor = vec4 ( 1,1,1,1 );

  // create a material to use
  Material *mat = new Material;
  mat->ambient = vec4 ( 0.1, 0.1, 0.2, 1.0 );
  mat->diffuse = vec4 ( 0.5, 0.5, 0.1, 1.0 );
  mat->specular = vec4 ( 1.0, 1.0, 1.0, 1.0 );
  mat->shininess = 133.0;
  mat->program = mat->loadShaders ( "PhongShading" );

  // attach the material to the primitive
  scene->setMaterial ( mat );

  // set the instance as the scene root
  root = scene;

  // misc OpenGL state
  glClearColor (0.0, 0.0, 0.0, 1.0);
  glEnable(GL_DEPTH_TEST);

}


void 
mouse ( int button, int state, int x, int y )
{
  if ( button == GLUT_LEFT_BUTTON && state == GLUT_DOWN ) {
    camera.startMouse ( x, height-y );
  }
}

void
motion ( int x, int y ) 
{
  camera.dragMouse(x,height-y);
}


int main(int argc, char** argv)
{
  glutInit(&argc, argv);
  glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH );
  glutInitWindowSize (300, 300); 
  glutInitWindowPosition (100, 100);
  glutCreateWindow (argv[0]);
  
#ifndef __APPLE__
  glewInit();
#endif
  init(argc,argv);
  
  glutDisplayFunc ( display );
  glutReshapeFunc ( reshape );
  glutKeyboardFunc ( keyboard );
  glutMouseFunc ( mouse );
  glutMotionFunc ( motion );
  glutTimerFunc( 33,timer,33 ); 
  glutMainLoop();
  return 0;
}


