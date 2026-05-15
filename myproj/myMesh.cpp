#include "myMesh.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <map>
#include <utility>
#include <GL/glew.h>
#include "myvector3d.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace std;

myMesh::myMesh(void)
{
	/**** TODO ****/
}


myMesh::~myMesh(void)
{
	// Libérer la mémoire allouée
	clear();
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

bool myMesh::verifyHalfEdgeStructure()
{
	bool isCorrect = true;
	int erreurs = 0;

	cout << "DEBUT DE LA VERIFICATION" << endl;

	// Vérification des demi-arêtes
	for (unsigned int i = 0; i < halfedges.size(); i++) {
		myHalfedge* h = halfedges[i];
		if (h == NULL) continue;

		// Vérification des pointeurs nuls
		if (h->source == NULL) { cout << "Erreur " << i << " : Pas de 'source' (sommet)" << endl; isCorrect = false; erreurs++; }
		if (h->adjacent_face == NULL) { cout << "Erreur " << i << " : Pas de 'adjacent_face'" << endl; isCorrect = false; erreurs++; }
		if (h->next == NULL) { cout << "Erreur " << i << " : Pas de 'next'" << endl; isCorrect = false; erreurs++; }
		if (h->prev == NULL) { cout << "Erreur " << i << " : Pas de 'prev'" << endl; isCorrect = false; erreurs++; }
		if (h->twin == NULL) { cout << "Erreur " << i << " : Pas de 'twin'" << endl; isCorrect = false; erreurs++; }

		// Vérification réciprocité
		if (h->twin != NULL && h->twin->twin != h) {
			cout << "Erreur " << i << " : Le twin de mon twin n'est pas moi-même " << endl; isCorrect = false; erreurs++;
		}
		if (h->next != NULL && h->next->prev != h) {
			cout << "Erreur " << i << " : Le prev de mon next n'est pas moi-même " << endl; isCorrect = false; erreurs++;
		}
		if (h->prev != NULL && h->prev->next != h) {
			cout << "Erreur " << i << " : Le next de mon prev n'est pas moi-même " << endl; isCorrect = false; erreurs++;
		}
	}

	// Vérification des sommets
	for (unsigned int i = 0; i < vertices.size(); i++) {
		myVertex* v = vertices[i];
		if (v == NULL) continue;

		// vérifier seulement que si originof existe, il pointe vers le bon vertex
		// originof == NULL est acceptable pour les maillages avec bords ou sommets inutilisés
		if (v->originof != NULL && v->originof->source != v) {
			cout << "Erreur [Sommet " << i << "] : Son 'originof' a une source qui est un autre sommet" << endl; 
			isCorrect = false; 
			erreurs++;
		}
	}

	// Vérification des faces
	for (unsigned int i = 0; i < faces.size(); i++) {
		myFace* f = faces[i];
		if (f == NULL) continue;

		if (f->adjacent_halfedge == NULL) {
			cout << "Erreur [Face " << i << "] : Ne pointe vers aucune demi-arete (adjacent_halfedge est NULL)" << endl; isCorrect = false; erreurs++;
		}
		else {
			if (f->adjacent_halfedge->adjacent_face != f) {
				cout << "Erreur [Face " << i << "] : Son 'adjacent_halfedge' pointe vers une autre face " << endl; isCorrect = false; erreurs++;
			}

			// Parcourir le contour de la face pour voir s'il est bien fermé
			myHalfedge* curr = f->adjacent_halfedge;
			int countAretes = 0;
			do {
				if (curr->adjacent_face != f) {
					cout << "Erreur [Face " << i << "] : Une demi-arete du contour ne pointe pas vers cette face" << endl; isCorrect = false; erreurs++;
				}
				curr = curr->next;
				countAretes++;

				// Sécurité anti-boucle infinie
				if (countAretes > 1000) {
					cout << "Erreur Fatale [Face " << i << "] : Boucle infinie detectee ! Le contour ne se referme jamais" << endl; isCorrect = false; erreurs++; break;
				}
			} while (curr != f->adjacent_halfedge && curr != NULL);

			if (countAretes < 3) {
				cout << "Erreur [Face " << i << "] : Face degeneree avec moins de 3 cotes (" << countAretes << " cotes)" << endl; isCorrect = false; erreurs++;
			}
		}
	}

	if (isCorrect) {
		cout << "-> Validation REUSSIE" << endl;
	}
	else {
		cout << "-> Validation ECHOUEE : " << erreurs << " erreurs trouvees" << endl;
	}
	cout << "FIN DE LA VERIFICATION" << endl << endl;

	return isCorrect;
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

	this->verifyHalfEdgeStructure();
}

//return false if already triangle, true othewise.
//bool myMesh::triangulate(myFace* f)
//{
//	// vérifier que la face et sa demi-arête existent
//	if (f == NULL || f->adjacent_halfedge == NULL) return false;
//
//	// récupérer toutes les demi-arêtes originales de la face dans l'ordre
//	vector<myHalfedge*> face_edges;
//	myHalfedge* curr = f->adjacent_halfedge;
//	do {
//		face_edges.push_back(curr);
//		curr = curr->next;
//	} while (curr != f->adjacent_halfedge && curr != NULL); // anti-boucle infinie
//
//	int n = face_edges.size();
//	if (n <= 3) return false; // si déjà un triangle -> false
//
//	// préparer les faces
//	// un polygone à N sommets donne (N - 2) triangles.
//	// le premier triangle réutilise la face 'f', on crée les autres.
//	vector<myFace*> tri_faces;
//	tri_faces.push_back(f);
//	for (int i = 0; i < n - 3; i++) {
//		myFace* newF = new myFace();
//		faces.push_back(newF);
//		tri_faces.push_back(newF);
//	}
//
//	// préparer les diagonales
//	// il nous faut (N - 3) paires de demi-arêtes internes pour couper le polygone.
//	vector<myHalfedge*> diag_out(n - 3); // diagonales qui partent du sommet 0
//	vector<myHalfedge*> diag_in(n - 3);  // diagonales qui reviennent vers le sommet 0
//
//	for (int i = 0; i < n - 3; i++) {
//		diag_out[i] = new myHalfedge();
//		diag_in[i] = new myHalfedge();
//
//		// connexion des twins
//		diag_out[i]->twin = diag_in[i];
//		diag_in[i]->twin = diag_out[i];
//
//		// assignation des sources (le sommet 0 est notre pivot)
//		diag_out[i]->source = face_edges[0]->source;
//		diag_in[i]->source = face_edges[i + 2]->source;
//
//		// ajout au maillage global
//		halfedges.push_back(diag_out[i]);
//		halfedges.push_back(diag_in[i]);
//	}
//
//	// connecter tous les triangles ensemble
//	for (int i = 0; i < n - 2; i++) {
//		myFace* current_face = tri_faces[i];
//
//		// chaque triangle est formé de 3 arêtes : eA, eB, eC
//		myHalfedge* eA; // arête sortant du sommet 0
//		myHalfedge* eB = face_edges[i + 1]; // l'arête originale du bord du polygone
//		myHalfedge* eC; // arête retournant vers le sommet 0
//
//		// déterminer eA
//		if (i == 0) {
//			eA = face_edges[0]; // pour le 1er triangle, c'est la 1ère arête originale
//		}
//		else {
//			eA = diag_out[i - 1]; // pour les autres, c'est la diagonale sortante
//		}
//
//		// déterminer eC
//		if (i == n - 3) {
//			eC = face_edges[n - 1]; // pour le dernier triangle, c'est la dernière arête originale
//		}
//		else {
//			eC = diag_in[i]; // pour les autres, c'est la diagonale entrante
//		}
//
//		// chaînage
//		eA->next = eB; eB->prev = eA;
//		eB->next = eC; eC->prev = eB;
//		eC->next = eA; eA->prev = eC;
//
//		// assigner la face courante aux 3 demi-arêtes
//		eA->adjacent_face = current_face;
//		eB->adjacent_face = current_face;
//		eC->adjacent_face = current_face;
//
//		// assigner une arête de référence à la face
//		current_face->adjacent_halfedge = eA;
//	}
//
//	return true;
//}

//triangulate with ear clipping
bool myMesh::triangulate(myFace* f)
{
	if (f == NULL || f->adjacent_halfedge == NULL) return false;


	vector<myHalfedge*> edges;
	myHalfedge* curr = f->adjacent_halfedge;
	do {
		edges.push_back(curr);
		curr = curr->next;
	} while (curr != f->adjacent_halfedge && curr != NULL);


	if (edges.size() <= 3) return false;


	// Calcul de la normale du polygone (Méthode de Newell)
	myVector3D normal(0, 0, 0);
	for (size_t i = 0; i < edges.size(); i++) {
		myPoint3D* p1 = edges[i]->source->point;
		myPoint3D* p2 = edges[(i + 1) % edges.size()]->source->point;
		normal.dX += (p1->Y - p2->Y) * (p1->Z + p2->Z);
		normal.dY += (p1->Z - p2->Z) * (p1->X + p2->X);
		normal.dZ += (p1->X - p2->X) * (p1->Y + p2->Y);
	}
	normal.normalize();


	while (edges.size() > 3) {
		bool earFound = false;
		int n = edges.size();


		for (int i = 0; i < n; i++) {
			int prev = (i - 1 + n) % n;
			int next = (i + 1) % n;


			// Environment = [Vi-1;Vi; Vi+1]
			myPoint3D* Vi_minus_1 = edges[prev]->source->point;
			myPoint3D* Vi = edges[i]->source->point;
			myPoint3D* Vi_plus_1 = edges[next]->source->point;


			// if Vi is convexe :
			myVector3D u = *Vi - *Vi_minus_1;
			myVector3D v = *Vi_plus_1 - *Vi;
			if ((u.crossproduct(v) * normal) > 1e-5) {


				// if has no vertex inside :
				bool has_no_vertex_inside = true;
				for (int j = 0; j < n; j++) {
					if (j == prev || j == i || j == next) continue;
					myPoint3D* p = edges[j]->source->point;


					myVector3D u1 = *Vi - *Vi_minus_1;
					myVector3D v1 = *p - *Vi_minus_1;
					myVector3D u2 = *Vi_plus_1 - *Vi;
					myVector3D v2 = *p - *Vi;
					myVector3D u3 = *Vi_minus_1 - *Vi_plus_1;
					myVector3D v3 = *p - *Vi_plus_1;


					if ((u1.crossproduct(v1) * normal) >= -1e-5 &&
						(u2.crossproduct(v2) * normal) >= -1e-5 &&
						(u3.crossproduct(v3) * normal) >= -1e-5) {
						has_no_vertex_inside = false;
						break;
					}
				}


				if (has_no_vertex_inside) {
					// clip Vi+1 and Vi-1
					myHalfedge* e_prev = edges[prev];
					myHalfedge* e_curr = edges[i];


					myHalfedge* diag_in = new myHalfedge();
					myHalfedge* diag_out = new myHalfedge();
					diag_in->twin = diag_out;
					diag_out->twin = diag_in;


					diag_in->source = edges[next]->source;
					diag_out->source = edges[prev]->source;


					halfedges.push_back(diag_in);
					halfedges.push_back(diag_out);


					myFace* newFace = new myFace();
					faces.push_back(newFace);
					newFace->adjacent_halfedge = e_prev;


					e_prev->next = e_curr;   e_curr->prev = e_prev;
					e_curr->next = diag_in;  diag_in->prev = e_curr;
					diag_in->next = e_prev;  e_prev->prev = diag_in;


					e_prev->adjacent_face = newFace;
					e_curr->adjacent_face = newFace;
					diag_in->adjacent_face = newFace;


					diag_out->next = edges[next];
					diag_out->prev = edges[(prev - 1 + n) % n];
					edges[next]->prev = diag_out;
					edges[(prev - 1 + n) % n]->next = diag_out;


					diag_out->adjacent_face = f;
					f->adjacent_halfedge = diag_out;


					// remove Vi
					edges[prev] = diag_out;
					edges.erase(edges.begin() + i);


					earFound = true;
					break;
				}
			}
		}


		if (!earFound) {
			cout << "Attention : Poly concavite bloquee." << endl;
			break;
		}
	}


	if (edges.size() == 3) {
		edges[0]->next = edges[1]; edges[1]->prev = edges[0];
		edges[1]->next = edges[2]; edges[2]->prev = edges[1];
		edges[2]->next = edges[0]; edges[0]->prev = edges[2];


		edges[0]->adjacent_face = f;
		edges[1]->adjacent_face = f;
		edges[2]->adjacent_face = f;
		f->adjacent_halfedge = edges[0];
	}

	this->verifyHalfEdgeStructure();	
	return true;
}




void lierDemiArete(myHalfedge* h, myVertex* a, myVertex* b, std::map<std::pair<myVertex*, myVertex*>, myHalfedge*>& edgeMap) {
	std::pair<myVertex*, myVertex*> twinKey = std::make_pair(b, a);
	std::map<std::pair<myVertex*, myVertex*>, myHalfedge*>::iterator it = edgeMap.find(twinKey);

	if (it != edgeMap.end()) {
		h->twin = it->second;
		it->second->twin = h;
	}
	else {
		edgeMap[std::make_pair(a, b)] = h;
	}
}

void creerTriangle(myMesh* m, myVertex* v1, myVertex* v2, myVertex* v3, std::map<std::pair<myVertex*, myVertex*>, myHalfedge*>& edgeMap) {
	// ignorer triangles degeneres
	if (v1 == NULL || v2 == NULL || v3 == NULL) return;
	if (v1 == v2 || v2 == v3 || v3 == v1) return;

	// allouer face et demi-aretes
	myFace* f = new myFace();
	myHalfedge* h1 = new myHalfedge();
	myHalfedge* h2 = new myHalfedge();
	myHalfedge* h3 = new myHalfedge();

	m->faces.push_back(f);
	m->halfedges.push_back(h1);
	m->halfedges.push_back(h2);
	m->halfedges.push_back(h3);

	// assigner sources
	h1->source = v1; h2->source = v2; h3->source = v3;

	// originof: ne pas ecraser s'il existe deja
	if (v1->originof == NULL) v1->originof = h1;
	if (v2->originof == NULL) v2->originof = h2;
	if (v3->originof == NULL) v3->originof = h3;

	// chainage
	h1->next = h2; h2->next = h3; h3->next = h1;
	h1->prev = h3; h2->prev = h1; h3->prev = h2;

	// liaisons face et demi-aretes
	h1->adjacent_face = f; h2->adjacent_face = f; h3->adjacent_face = f;
	f->adjacent_halfedge = h1;

	// liaison des twins
	lierDemiArete(h1, v1, v2, edgeMap);
	lierDemiArete(h2, v2, v3, edgeMap);
	lierDemiArete(h3, v3, v1, edgeMap);
}


//surface of revolution
bool myMesh::generateRevolutionMesh(const std::vector<std::pair<double, double> >& profile, int slices) {
	
	this->clear();

	int numPoints = profile.size();
	if (numPoints < 2 || slices < 3) {
		cout << "Erreur : profil invalide ou trop peu de slices." << endl;
		return false;
	}

	// création des sommets
	for (int i = 0; i < numPoints; ++i) {
		double r = profile[i].first;
		double z = profile[i].second;

		for (int j = 0; j < slices; ++j) {
			double theta = (2.0 * M_PI * j) / slices;
			double x = r * cos(theta);
			double y = r * sin(theta);

			myVertex* v = new myVertex();
			v->point = new myPoint3D(x, y, z);
			v->originof = NULL;
			this->vertices.push_back(v);
		}
	}

	// map pour lier les demi-aretes jumelles (twins)
	std::map<std::pair<myVertex*, myVertex*>, myHalfedge*> edgeMap;

	// parcours des quads (entre anneaux) et creation de deux triangles par quad
	for (int i = 0; i < numPoints - 1; ++i) {
		for (int j = 0; j < slices; ++j) {
			int j_next = (j + 1) % slices;

			myVertex* v00 = this->vertices[i * slices + j];
			myVertex* v10 = this->vertices[(i + 1) * slices + j];
			myVertex* v11 = this->vertices[(i + 1) * slices + j_next];
			myVertex* v01 = this->vertices[i * slices + j_next];

			// deux triangles couvrant le quad
			creerTriangle(this, v00, v10, v11, edgeMap);
			creerTriangle(this, v00, v11, v01, edgeMap);
		}
	}

	// calcul des normales avec des boucles for classiques
	for (unsigned int i = 0; i < faces.size(); ++i) {
		if (faces[i]) faces[i]->computeNormal();
	}

	for (unsigned int i = 0; i < vertices.size(); ++i) {
		if (vertices[i]) vertices[i]->computeNormal();
	}

	cout << "Surface de revolution generee avec succes !" << endl;

	this->verifyHalfEdgeStructure();

	return true;
}