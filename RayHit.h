struct RayHit{
	bool hit;
	Vector3d point;
	Vector3d normal;
	double distance;
	long oindex;
	double uv[2];
	Material *mat;
	
};

