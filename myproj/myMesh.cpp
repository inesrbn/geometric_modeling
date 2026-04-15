#include "myMesh.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <map>
#include <utility>
#include <GL/glew.h>
#include "myvector3d.h"

using namespace std;

myMesh::myMesh(void)
{
	/**** TODO ****/
}


myMesh::~myMesh(void)
{
	/**** TODO ****/
}

void myMesh::clear()
{
	for (unsigned int i = 0; i < vertices.size(); i++) if (vertices[i]) delete vertices[i];
	for (unsigned int i = 0; i < halfedges.size(); i++) if (halfedges[i]) delete halfedges[i];
	for (unsigned int i = 0; i < faces.size(); i++) if (faces[i]) delete faces[i];

	vector<myVertex*> empty_vertices;    vertices.swap(empty_vertices);
	vector<myHalfedge*> empty_halfedges; halfedges.swap(empty_halfedges);
	vector<myFace*> empty_faces;         faces.swap(empty_faces);
}

void myMesh::checkMesh()
{
	vector<myHalfedge*>::iterator it;
	for (it = halfedges.begin(); it != halfedges.end(); it++)
	{
		if ((*it)->twin == NULL)
			break;
	}
	if (it != halfedges.end())
		cout << "Error! Not all edges have their twins!\n";
	else cout << "Each edge has a twin!\n";
}


bool myMesh::readFile(std::string filename)
{
	string s, t, u;
	vector<int> faceids;
	myHalfedge** hedges;

	ifstream fin(filename);
	if (!fin.is_open()) {
		cout << "Unable to open file!\n";
		return false;
	}
	name = filename;

	map<pair<int, int>, myHalfedge*> twin_map;
	map<pair<int, int>, myHalfedge*>::iterator it;

	while (getline(fin, s))
	{
		stringstream myline(s);
		myline >> t;
		if (t == "g") {}
		else if (t == "v")
		{
			float x, y, z;
			myline >> x >> y >> z;

			// Instanciation du sommet et de ses coordonnées
			myVertex* v = new myVertex();
			v->point = new myPoint3D(x, y, z);
			vertices.push_back(v);
		}
		else if (t == "mtllib") {}
		else if (t == "usemtl") {}
		else if (t == "s") {}
		else if (t == "f")
		{
			faceids.clear();
			while (myline >> u) {
				// Lit les indices des sommets de la face (les indices OBJ commencent à 1, donc on fait -1)
				faceids.push_back(atoi((u.substr(0, u.find("/"))).c_str()) - 1);
			}
			if (faceids.size() < 3) {
				// Ignore les faces dégénérées
				continue;
			}

			// Alloue le tableau pour stocker les pointeurs vers les half-edges
			hedges = new myHalfedge * [faceids.size()];
			for (unsigned int i = 0; i < faceids.size(); i++) {
				hedges[i] = new myHalfedge(); // Pré-alloue les nouveaux half-edges
			}

			// Alloue la nouvelle face et la connecte à sa première demi-arête
			myFace* f = new myFace();
			f->adjacent_halfedge = hedges[0];

			for (unsigned int i = 0; i < faceids.size(); i++)
			{
				int iplusone = (i + 1) % faceids.size();
				int iminusone = (i - 1 + faceids.size()) % faceids.size();

				// Connexion de la demi-arête avec la face, les sommets et les arêtes voisines
				hedges[i]->next = hedges[iplusone];
				hedges[i]->prev = hedges[iminusone];
				hedges[i]->adjacent_face = f;
				hedges[i]->source = vertices[faceids[i]];

				// Si le sommet n'a pas encore de demi-arête d'origine, on lui assigne celle-ci
				if (vertices[faceids[i]]->originof == NULL) {
					vertices[faceids[i]]->originof = hedges[i];
				}

				// Recherche des arêtes jumelles (twins) via la map
				pair<int, int> current_edge = make_pair(faceids[i], faceids[iplusone]);
				pair<int, int> twin_edge = make_pair(faceids[iplusone], faceids[i]);

				it = twin_map.find(twin_edge);
				if (it != twin_map.end()) {
					// Une arête jumelle a été trouvée, on les connecte mutuellement
					hedges[i]->twin = it->second;
					it->second->twin = hedges[i];
				}
				else {
					// Pas encore de jumelle, on ajoute la demi-arête actuelle à la map
					twin_map[current_edge] = hedges[i];
				}

				// Ajoute la demi-arête à la liste globale du maillage
				halfedges.push_back(hedges[i]);
			}
			delete[] hedges; // Libère le tableau de pointeurs temporaire

			// Ajoute la face à la liste globale du maillage
			faces.push_back(f);
		}
	}

	checkMesh();
	normalize();

	return true;
}


void myMesh::computeNormals()
{
	// Étape A : Calculer la normale de chaque face
	for (unsigned int i = 0; i < faces.size(); i++) {
		if (faces[i]) faces[i]->computeNormal();
	}

	// Étape B : Calculer la normale de chaque sommet 
	// (Dépend de l'étape A car utilise faces[i]->normal)
	for (unsigned int i = 0; i < vertices.size(); i++) {
		if (vertices[i]) vertices[i]->computeNormal();
	}
}

void myMesh::normalize()
{
	if (vertices.size() < 1) return;

	int tmpxmin = 0, tmpymin = 0, tmpzmin = 0, tmpxmax = 0, tmpymax = 0, tmpzmax = 0;

	for (unsigned int i = 0; i < vertices.size(); i++) {
		if (vertices[i]->point->X < vertices[tmpxmin]->point->X) tmpxmin = i;
		if (vertices[i]->point->X > vertices[tmpxmax]->point->X) tmpxmax = i;

		if (vertices[i]->point->Y < vertices[tmpymin]->point->Y) tmpymin = i;
		if (vertices[i]->point->Y > vertices[tmpymax]->point->Y) tmpymax = i;

		if (vertices[i]->point->Z < vertices[tmpzmin]->point->Z) tmpzmin = i;
		if (vertices[i]->point->Z > vertices[tmpzmax]->point->Z) tmpzmax = i;
	}

	double xmin = vertices[tmpxmin]->point->X, xmax = vertices[tmpxmax]->point->X,
		ymin = vertices[tmpymin]->point->Y, ymax = vertices[tmpymax]->point->Y,
		zmin = vertices[tmpzmin]->point->Z, zmax = vertices[tmpzmax]->point->Z;

	double scale = (xmax - xmin) > (ymax - ymin) ? (xmax - xmin) : (ymax - ymin);
	scale = scale > (zmax - zmin) ? scale : (zmax - zmin);

	for (unsigned int i = 0; i < vertices.size(); i++) {
		vertices[i]->point->X -= (xmax + xmin) / 2;
		vertices[i]->point->Y -= (ymax + ymin) / 2;
		vertices[i]->point->Z -= (zmax + zmin) / 2;

		vertices[i]->point->X /= scale;
		vertices[i]->point->Y /= scale;
		vertices[i]->point->Z /= scale;
	}
}


void myMesh::splitFaceTRIS(myFace* f, myPoint3D* p)
{
	/**** TODO ****/
}

void myMesh::splitEdge(myHalfedge* e1, myPoint3D* p)
{

	/**** TODO ****/
}

void myMesh::splitFaceQUADS(myFace* f, myPoint3D* p)
{
	/**** TODO ****/
}


void myMesh::subdivisionCatmullClark()
{
	/**** TODO ****/
}


void myMesh::triangulate()
{
	// on copie la liste actuelle pour ne pas boucler à l'infini sur les nouveaux triangles
	vector<myFace*> original_faces = faces;

	for (unsigned int i = 0; i < original_faces.size(); i++) {
		// la fonction triangulate(f) s'occupe de la découpe en éventail
		triangulate(original_faces[i]);
	}

	// mise à jour finale des normales
	computeNormals();
	cout << "Maillage triangule avec succes." << endl;
}

//return false if already triangle, true othewise.
bool myMesh::triangulate(myFace* f)
{
	// vérifier que la face et sa demi-arête existent
	if (f == NULL || f->adjacent_halfedge == NULL) return false;

	// récupérer toutes les demi-arêtes originales de la face dans l'ordre
	vector<myHalfedge*> face_edges;
	myHalfedge* curr = f->adjacent_halfedge;
	do {
		face_edges.push_back(curr);
		curr = curr->next;
	} while (curr != f->adjacent_halfedge && curr != NULL); // anti-boucle infinie

	int n = face_edges.size();
	if (n <= 3) return false; // si déjà un triangle -> false

	// préparer les faces
	// un polygone à N sommets donne (N - 2) triangles.
	// le premier triangle réutilise la face 'f', on crée les autres.
	vector<myFace*> tri_faces;
	tri_faces.push_back(f);
	for (int i = 0; i < n - 3; i++) {
		myFace* newF = new myFace();
		faces.push_back(newF);
		tri_faces.push_back(newF);
	}

	// préparer les diagonales
	// il nous faut (N - 3) paires de demi-arêtes internes pour couper le polygone.
	vector<myHalfedge*> diag_out(n - 3); // diagonales qui partent du sommet 0
	vector<myHalfedge*> diag_in(n - 3);  // diagonales qui reviennent vers le sommet 0

	for (int i = 0; i < n - 3; i++) {
		diag_out[i] = new myHalfedge();
		diag_in[i] = new myHalfedge();

		// connexion des twins
		diag_out[i]->twin = diag_in[i];
		diag_in[i]->twin = diag_out[i];

		// assignation des sources (le sommet 0 est notre pivot)
		diag_out[i]->source = face_edges[0]->source;
		diag_in[i]->source = face_edges[i + 2]->source;

		// ajout au maillage global
		halfedges.push_back(diag_out[i]);
		halfedges.push_back(diag_in[i]);
	}

	// connecter tous les triangles ensemble
	for (int i = 0; i < n - 2; i++) {
		myFace* current_face = tri_faces[i];

		// chaque triangle est formé de 3 arêtes : eA, eB, eC
		myHalfedge* eA; // arête sortant du sommet 0
		myHalfedge* eB = face_edges[i + 1]; // l'arête originale du bord du polygone
		myHalfedge* eC; // arête retournant vers le sommet 0

		// déterminer eA
		if (i == 0) {
			eA = face_edges[0]; // pour le 1er triangle, c'est la 1ère arête originale
		}
		else {
			eA = diag_out[i - 1]; // pour les autres, c'est la diagonale sortante
		}

		// déterminer eC
		if (i == n - 3) {
			eC = face_edges[n - 1]; // pour le dernier triangle, c'est la dernière arête originale
		}
		else {
			eC = diag_in[i]; // pour les autres, c'est la diagonale entrante
		}

		// chaînage
		eA->next = eB; eB->prev = eA;
		eB->next = eC; eC->prev = eB;
		eC->next = eA; eA->prev = eC;

		// assigner la face courante aux 3 demi-arêtes
		eA->adjacent_face = current_face;
		eB->adjacent_face = current_face;
		eC->adjacent_face = current_face;

		// assigner une arête de référence à la face
		current_face->adjacent_halfedge = eA;
	}

	return true;
}
