#include "myVertex.h"
#include "myvector3d.h"
#include "myHalfedge.h"
#include "myFace.h"

myVertex::myVertex(void)
{
	point = NULL;
	originof = NULL;
	normal = new myVector3D(1.0,1.0,1.0);
}

myVertex::~myVertex(void)
{
	if (normal) delete normal;
}

void myVertex::computeNormal()
{
    myHalfedge* step = originof;
    normal->dX = normal->dY = normal->dZ = 0;
    int counter = 0;

    if (step == NULL) return;

    do {
        if (step->adjacent_face != NULL) {
            // On ajoute la normale de la face adjacente
            *normal += *(step->adjacent_face->normal);
            counter++;
        }
        // Navigation vers la demi-arête suivante autour du sommet (sens horaire ou anti-horaire)
        // On prend le twin de la précédente (prev) pour changer de face
        step = step->prev->twin;

    } while (step != originof && step != NULL);

    if (counter > 0) {
        // On fait la moyenne et on normalise le vecteur final
        normal->dX /= counter;
        normal->dY /= counter;
        normal->dZ /= counter;
        normal->normalize();
    }
}
