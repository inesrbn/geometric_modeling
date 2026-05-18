#pragma once
#include "myFace.h"
#include "myHalfedge.h"
#include "myVertex.h"
#include <vector>
#include <string>
#include <utility>

class myMesh
{
public:
	std::vector<myVertex *> vertices;
	std::vector<myHalfedge *> halfedges;
	std::vector<myFace *> faces;
	std::string name;

	void checkMesh();
	bool verifyHalfEdgeStructure();
	bool readFile(std::string filename);
	void computeNormals();
	void normalize();

	void subdivisionCatmullClark();

	void splitFaceTRIS(myFace *, myPoint3D *);

	void splitEdge(myHalfedge *, myPoint3D *);
	void splitFaceQUADS(myFace *, myPoint3D *);

	void triangulate();
	bool triangulate(myFace *);

	void clear();

	myMesh(void);
	~myMesh(void);

	bool generateRevolutionMesh(const std::vector<std::pair<double, double> >& profile, int slices);

	double edgeLength(myHalfedge* h);
	void collapseEdge(myHalfedge* h);
	void simplifyShortestEdgeCollapse(int iterations);
	bool canCollapse(myHalfedge* h);
};

