/*
//------------------------------------------------------------------
// Matrix::Matrix
//------------------------------------------------------------------
Matrix::Matrix(const Scale &scale)
: is_id(false), inv_ok(false)
{
    mat[1]=mat[2]=mat[3]=0.0f;
    mat[4]=mat[6]=mat[7]=0.0f;
    mat[8]=mat[9]=mat[11]=0.0f;
    mat[12]=mat[13]=mat[14]=0.0f;
	mat[15]=1.0f;

	mat[0]=scale.x;
	mat[5]=scale.y;
	mat[10]=scale.z;
}




//------------------------------------------------------------------
// Matrix::Matrix
//------------------------------------------------------------------
Matrix::Matrix(const Plane &plane)
: is_id(false), inv_ok(false)
{
	// Create a mirror matrix
	mat[0]=1-2*plane.planenormal.x*plane.planenormal.x;
	mat[1]=-2*plane.planenormal.x*plane.planenormal.y;
	mat[2]=-2*plane.planenormal.x*plane.planenormal.z;

	mat[4]=-2*plane.planenormal.y*plane.planenormal.x;
	mat[5]=1-2*plane.planenormal.y*plane.planenormal.y;
	mat[6]=-2*plane.planenormal.y*plane.planenormal.z;

	mat[8]=-2*plane.planenormal.z*plane.planenormal.x;
	mat[9]=-2*plane.planenormal.z*plane.planenormal.y;
	mat[10]=1-2*plane.planenormal.z*plane.planenormal.z;

	mat[12]=-2*plane.range_to_origin*plane.planenormal.x;
	mat[13]=-2*plane.range_to_origin*plane.planenormal.y;
	mat[14]=-2*plane.range_to_origin*plane.planenormal.z;

	mat[2]=mat[7]=mat[11]=0;
	mat[15]=1;
}


//------------------------------------------------------------------
// Matrix::operator==
//------------------------------------------------------------------
bool Matrix::operator==(const Matrix &other) const
{
	if ((fabsf(mat[0]-other.mat[0])<EPSILON)&&
		(fabsf(mat[1]-other.mat[1])<EPSILON)&&
		(fabsf(mat[2]-other.mat[2])<EPSILON)&&
		(fabsf(mat[3]-other.mat[3])<EPSILON)&&
		(fabsf(mat[4]-other.mat[4])<EPSILON)&&
		(fabsf(mat[5]-other.mat[5])<EPSILON)&&
		(fabsf(mat[6]-other.mat[6])<EPSILON)&&
		(fabsf(mat[7]-other.mat[7])<EPSILON)&&
		(fabsf(mat[8]-other.mat[8])<EPSILON)&&
		(fabsf(mat[9]-other.mat[9])<EPSILON)&&
		(fabsf(mat[10]-other.mat[10])<EPSILON)&&
		(fabsf(mat[11]-other.mat[11])<EPSILON)&&
		(fabsf(mat[12]-other.mat[12])<EPSILON)&&
		(fabsf(mat[13]-other.mat[13])<EPSILON)&&
		(fabsf(mat[14]-other.mat[14])<EPSILON)&&
		(fabsf(mat[15]-other.mat[15])<EPSILON)) return true;
	return false;
}*/

