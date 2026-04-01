#include "myFace.h"
#include "myvector3d.h"
#include "myHalfedge.h"
#include "myVertex.h"
#include <GL/glew.h>

myFace::myFace(void)
{
	adjacent_halfedge = NULL;
	normal = new myVector3D(1.0, 1.0, 1.0);
}

myFace::~myFace(void)
{
	if (normal) delete normal;
}

void myFace::computeNormal()
{
    // On récupère 3 sommets consécutifs de la face pour créer deux vecteurs
    myPoint3D* p1 = adjacent_halfedge->source->point;
    myPoint3D* p2 = adjacent_halfedge->next->source->point;
    myPoint3D* p3 = adjacent_halfedge->next->next->source->point;

    // Vecteur v1 = p2 - p1
    myVector3D v1(p2->X - p1->X, p2->Y - p1->Y, p2->Z - p1->Z);
    // Vecteur v2 = p3 - p1
    myVector3D v2(p3->X - p1->X, p3->Y - p1->Y, p3->Z - p1->Z);

    // La normale est le produit vectoriel de v1 et v2
    *normal = v1.crossproduct(v2);

    // On n'oublie pas de normaliser pour que le vecteur ait une longueur de 1
    normal->normalize();
}
