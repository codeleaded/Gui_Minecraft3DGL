#ifndef MATH3D_H
#define MATH3D_H

#include "/home/codeleaded/System/Static/Container/Vector.h"

typedef struct vec2d{
	float u;
	float v;
	float w;
} vec2d;

vec2d vec2d_New(){
    return (vec2d){ 0.0f,0.0f,1.0f };
}
vec2d vec2d_Make(float u,float v){
    return (vec2d){ u,v,1.0f };
}


typedef struct vec3d{
	float x;
	float y;
	float z;
	float w;
} vec3d;

vec3d vec3d_New(){
    return (vec3d){ 0.0f,0.0f,0.0f,1.0f };
}
vec3d vec3d_Make(float x,float y,float z){
    return (vec3d){ x,y,z,1.0f };
}


typedef struct triangle{
	vec3d p[3];
	vec2d t[3];
	Pixel c;
	unsigned char id;
} triangle;


typedef struct mesh{
	Vector tris;
} mesh;

mesh mesh_New(){
    mesh m;
    m.tris = Vector_New(sizeof(triangle));
    return m;
}
void mesh_Free(mesh* m){
    Vector_Free(&m->tris);
}


typedef struct mat4x4{
	float m[4][4];
} mat4x4;

mat4x4 mat4x4_New(){
    mat4x4 m;
    memset(&m,0,sizeof(m));
    return m;
}


vec3d vec3d_Add(vec3d v1,vec3d v2){
	return vec3d_Make( v1.x + v2.x, v1.y + v2.y, v1.z + v2.z );
}
vec3d vec3d_Sub(vec3d v1,vec3d v2){
	return vec3d_Make( v1.x - v2.x, v1.y - v2.y, v1.z - v2.z );
}
vec3d vec3d_Mul(vec3d v1,float k){
	return vec3d_Make( v1.x * k, v1.y * k, v1.z * k );
}
vec3d vec3d_Div(vec3d v1,float k){
	return vec3d_Make( v1.x / k, v1.y / k, v1.z / k );
}
float vec3d_DotProduct(vec3d v1,vec3d v2){
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}
float vec3d_Length(vec3d v){
	return sqrtf(vec3d_DotProduct(v,v));
}
vec3d vec3d_Normalise(vec3d v){
	float l = vec3d_Length(v);
	return vec3d_Make( v.x / l, v.y / l, v.z / l );
}
vec3d vec3d_CrossProduct(vec3d v1,vec3d v2){
	vec3d v = vec3d_New();
	v.x = v1.y * v2.z - v1.z * v2.y;
	v.y = v1.z * v2.x - v1.x * v2.z;
	v.z = v1.x * v2.y - v1.y * v2.x;
	return v;
}
vec3d vec3d_IntersectPlane(vec3d plane_p,vec3d* plane_n,vec3d lineStart,vec3d lineEnd,float* t){
	*plane_n = vec3d_Normalise(*plane_n);
	float plane_d = -vec3d_DotProduct(*plane_n,plane_p);
	float ad = vec3d_DotProduct(lineStart,*plane_n);
	float bd = vec3d_DotProduct(lineEnd,*plane_n);
	*t = (-plane_d - ad) / (bd - ad);
	vec3d lineStartToEnd = vec3d_Sub(lineEnd, lineStart);
	vec3d lineToIntersect = vec3d_Mul(lineStartToEnd,*t);
	return vec3d_Add(lineStart,lineToIntersect);
}


vec3d Matrix_MultiplyVector(mat4x4 m,vec3d i){
	vec3d v = vec3d_New();
	v.x = i.x * m.m[0][0] + i.y * m.m[1][0] + i.z * m.m[2][0] + i.w * m.m[3][0];
	v.y = i.x * m.m[0][1] + i.y * m.m[1][1] + i.z * m.m[2][1] + i.w * m.m[3][1];
	v.z = i.x * m.m[0][2] + i.y * m.m[1][2] + i.z * m.m[2][2] + i.w * m.m[3][2];
	v.w = i.x * m.m[0][3] + i.y * m.m[1][3] + i.z * m.m[2][3] + i.w * m.m[3][3];
	return v;
}
mat4x4 Matrix_MakeIdentity(){
	mat4x4 matrix = mat4x4_New();
	matrix.m[0][0] = 1.0f;
	matrix.m[1][1] = 1.0f;
	matrix.m[2][2] = 1.0f;
	matrix.m[3][3] = 1.0f;
	return matrix;
}
mat4x4 Matrix_MakeRotationX(float fAngleRad){
	mat4x4 matrix = mat4x4_New();
	matrix.m[0][0] = 1.0f;
	matrix.m[1][1] = cosf(fAngleRad);
	matrix.m[1][2] = sinf(fAngleRad);
	matrix.m[2][1] = -sinf(fAngleRad);
	matrix.m[2][2] = cosf(fAngleRad);
	matrix.m[3][3] = 1.0f;
	return matrix;
}
mat4x4 Matrix_MakeRotationY(float fAngleRad){
	mat4x4 matrix = mat4x4_New();
	matrix.m[0][0] = cosf(fAngleRad);
	matrix.m[0][2] = sinf(fAngleRad);
	matrix.m[2][0] = -sinf(fAngleRad);
	matrix.m[1][1] = 1.0f;
	matrix.m[2][2] = cosf(fAngleRad);
	matrix.m[3][3] = 1.0f;
	return matrix;
}
mat4x4 Matrix_MakeRotationZ(float fAngleRad){
	mat4x4 matrix = mat4x4_New();
	matrix.m[0][0] = cosf(fAngleRad);
	matrix.m[0][1] = sinf(fAngleRad);
	matrix.m[1][0] = -sinf(fAngleRad);
	matrix.m[1][1] = cosf(fAngleRad);
	matrix.m[2][2] = 1.0f;
	matrix.m[3][3] = 1.0f;
	return matrix;
}
mat4x4 Matrix_MakeTranslation(float x, float y, float z){
	mat4x4 matrix = mat4x4_New();
	matrix.m[0][0] = 1.0f;
	matrix.m[1][1] = 1.0f;
	matrix.m[2][2] = 1.0f;
	matrix.m[3][3] = 1.0f;
	matrix.m[3][0] = x;
	matrix.m[3][1] = y;
	matrix.m[3][2] = z;
	return matrix;
}
mat4x4 Matrix_MakeProjection(float fFovDegrees, float fAspectRatio, float fNear, float fFar){
	float fFovRad = 1.0f / tanf(fFovDegrees * 0.5f / 180.0f * 3.14159f);
	
	mat4x4 matrix = mat4x4_New();
	matrix.m[0][0] = fAspectRatio * fFovRad;
	matrix.m[1][1] = fFovRad;
	matrix.m[2][2] = fFar / (fFar - fNear);
	matrix.m[3][2] = (-fFar * fNear) / (fFar - fNear);
	matrix.m[2][3] = 1.0f;
	matrix.m[3][3] = 0.0f;
	return matrix;
}
mat4x4 Matrix_MultiplyMatrix(mat4x4 m1, mat4x4 m2){
	mat4x4 matrix = mat4x4_New();
	for (int c = 0; c < 4; c++)
		for (int r = 0; r < 4; r++)
			matrix.m[r][c] = m1.m[r][0] * m2.m[0][c] + m1.m[r][1] * m2.m[1][c] + m1.m[r][2] * m2.m[2][c] + m1.m[r][3] * m2.m[3][c];
	return matrix;
}
mat4x4 Matrix_PointAt(vec3d pos,vec3d target,vec3d up){
	vec3d newForward = vec3d_Sub(target, pos);
	newForward = vec3d_Normalise(newForward);
	
    vec3d a = vec3d_Mul(newForward,vec3d_DotProduct(up,newForward));
	vec3d newUp = vec3d_Sub(up, a);
	newUp = vec3d_Normalise(newUp);
	vec3d newRight = vec3d_CrossProduct(newUp, newForward);
	
    mat4x4 matrix = mat4x4_New();
	matrix.m[0][0] = newRight.x;	matrix.m[0][1] = newRight.y;	matrix.m[0][2] = newRight.z;	matrix.m[0][3] = 0.0f;
	matrix.m[1][0] = newUp.x;		matrix.m[1][1] = newUp.y;		matrix.m[1][2] = newUp.z;		matrix.m[1][3] = 0.0f;
	matrix.m[2][0] = newForward.x;	matrix.m[2][1] = newForward.y;	matrix.m[2][2] = newForward.z;	matrix.m[2][3] = 0.0f;
	matrix.m[3][0] = pos.x;			matrix.m[3][1] = pos.y;			matrix.m[3][2] = pos.z;			matrix.m[3][3] = 1.0f;
	return matrix;
}
mat4x4 Matrix_QuickInverse(mat4x4 m){
	mat4x4 matrix = mat4x4_New();
	matrix.m[0][0] = m.m[0][0]; matrix.m[0][1] = m.m[1][0]; matrix.m[0][2] = m.m[2][0]; matrix.m[0][3] = 0.0f;
	matrix.m[1][0] = m.m[0][1]; matrix.m[1][1] = m.m[1][1]; matrix.m[1][2] = m.m[2][1]; matrix.m[1][3] = 0.0f;
	matrix.m[2][0] = m.m[0][2]; matrix.m[2][1] = m.m[1][2]; matrix.m[2][2] = m.m[2][2]; matrix.m[2][3] = 0.0f;
	matrix.m[3][0] = -(m.m[3][0] * matrix.m[0][0] + m.m[3][1] * matrix.m[1][0] + m.m[3][2] * matrix.m[2][0]);
	matrix.m[3][1] = -(m.m[3][0] * matrix.m[0][1] + m.m[3][1] * matrix.m[1][1] + m.m[3][2] * matrix.m[2][1]);
	matrix.m[3][2] = -(m.m[3][0] * matrix.m[0][2] + m.m[3][1] * matrix.m[1][2] + m.m[3][2] * matrix.m[2][2]);
	matrix.m[3][3] = 1.0f;
	return matrix;
}

float dist(vec3d p,vec3d plane_p,vec3d plane_n){
    vec3d n = vec3d_Normalise(p);
	return (plane_n.x * p.x + plane_n.y * p.y + plane_n.z * p.z - vec3d_DotProduct(plane_n, plane_p));
}

int Triangle_ClipAgainstPlane(vec3d plane_p,vec3d plane_n,triangle* in_tri,triangle* out_tri1,triangle* out_tri2){
	plane_n = vec3d_Normalise(plane_n);
	
	vec3d* inside_points[3];  int nInsidePointCount = 0;
	vec3d* outside_points[3]; int nOutsidePointCount = 0;
	vec2d* inside_tex[3]; int nInsideTexCount = 0;
	vec2d* outside_tex[3]; int nOutsideTexCount = 0;
	
    float d0 = dist(in_tri->p[0],plane_p,plane_n);
	float d1 = dist(in_tri->p[1],plane_p,plane_n);
	float d2 = dist(in_tri->p[2],plane_p,plane_n);
	if (d0 >= 0) { inside_points[nInsidePointCount++] = &in_tri->p[0]; inside_tex[nInsideTexCount++] = &in_tri->t[0]; }
	else {
		outside_points[nOutsidePointCount++] = &in_tri->p[0]; outside_tex[nOutsideTexCount++] = &in_tri->t[0];
	}
	if (d1 >= 0) {
		inside_points[nInsidePointCount++] = &in_tri->p[1]; inside_tex[nInsideTexCount++] = &in_tri->t[1];
	}
	else {
		outside_points[nOutsidePointCount++] = &in_tri->p[1];  outside_tex[nOutsideTexCount++] = &in_tri->t[1];
	}
	if (d2 >= 0) {
		inside_points[nInsidePointCount++] = &in_tri->p[2]; inside_tex[nInsideTexCount++] = &in_tri->t[2];
	}
	else {
		outside_points[nOutsidePointCount++] = &in_tri->p[2];  outside_tex[nOutsideTexCount++] = &in_tri->t[2];
	}
	
	if (nInsidePointCount == 0){
		return 0;
	}
	if (nInsidePointCount == 3){
		*out_tri1 = *in_tri;
		return 1;
	}
	if (nInsidePointCount == 1 && nOutsidePointCount == 2){
		out_tri1->c =  in_tri->c;
		out_tri1->id = in_tri->id;
		
        out_tri1->p[0] = *inside_points[0];
		out_tri1->t[0] = *inside_tex[0];
		
		float t;
		out_tri1->p[1] = vec3d_IntersectPlane(plane_p,&plane_n,*inside_points[0],*outside_points[0],&t);
		out_tri1->t[1].u = t * (outside_tex[0]->u - inside_tex[0]->u) + inside_tex[0]->u;
		out_tri1->t[1].v = t * (outside_tex[0]->v - inside_tex[0]->v) + inside_tex[0]->v;
		out_tri1->t[1].w = t * (outside_tex[0]->w - inside_tex[0]->w) + inside_tex[0]->w;
		out_tri1->p[2] = vec3d_IntersectPlane(plane_p,&plane_n,*inside_points[0],*outside_points[1],&t);
		out_tri1->t[2].u = t * (outside_tex[1]->u - inside_tex[0]->u) + inside_tex[0]->u;
		out_tri1->t[2].v = t * (outside_tex[1]->v - inside_tex[0]->v) + inside_tex[0]->v;
		out_tri1->t[2].w = t * (outside_tex[1]->w - inside_tex[0]->w) + inside_tex[0]->w;
		return 1;
	}
	if (nInsidePointCount == 2 && nOutsidePointCount == 1){
		out_tri1->c =  in_tri->c;
		out_tri1->id = in_tri->id;
		out_tri2->c =  in_tri->c;
		out_tri2->id = in_tri->id;
		
        out_tri1->p[0] = *inside_points[0];
		out_tri1->p[1] = *inside_points[1];
		out_tri1->t[0] = *inside_tex[0];
		out_tri1->t[1] = *inside_tex[1];
		
		float t;
		out_tri1->p[2] = vec3d_IntersectPlane(plane_p,&plane_n,*inside_points[0],*outside_points[0],&t);
		out_tri1->t[2].u = t * (outside_tex[0]->u - inside_tex[0]->u) + inside_tex[0]->u;
		out_tri1->t[2].v = t * (outside_tex[0]->v - inside_tex[0]->v) + inside_tex[0]->v;
		out_tri1->t[2].w = t * (outside_tex[0]->w - inside_tex[0]->w) + inside_tex[0]->w;
		
        out_tri2->p[0] = *inside_points[1];
		out_tri2->t[0] = *inside_tex[1];
		out_tri2->p[1] = out_tri1->p[2];
		out_tri2->t[1] = out_tri1->t[2];
		out_tri2->p[2] = vec3d_IntersectPlane(plane_p,&plane_n,*inside_points[1],*outside_points[0],&t);
		out_tri2->t[2].u = t * (outside_tex[0]->u - inside_tex[1]->u) + inside_tex[1]->u;
		out_tri2->t[2].v = t * (outside_tex[0]->v - inside_tex[1]->v) + inside_tex[1]->v;
		out_tri2->t[2].w = t * (outside_tex[0]->w - inside_tex[1]->w) + inside_tex[1]->w;
		return 2;
	}
}

#endif //!MATH3D_H