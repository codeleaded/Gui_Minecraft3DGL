#ifndef HITBOX3D_H
#define HITBOX3D_H

#include "./Math3D.h"

typedef struct Rect3 {
	vec3d p;
	vec3d d;
} Rect3;

char Rect3_Overlap(Rect3 r1,Rect3 r2){
	return !(r1.p.x<r2.p.x-r1.d.x || r1.p.y<r2.p.y-r1.d.y || r1.p.z<r2.p.z-r1.d.z || r1.p.x>r2.p.x+r2.d.x || r1.p.y>r2.p.y+r2.d.y || r1.p.z>r2.p.z+r2.d.z);
}
void Rect3_Static(Rect3* r1,Rect3 r2,void* Data,void (**Funcs)(void*)){
	if(Rect3_Overlap(*r1,r2)){
        vec3d m1 = vec3d_Add(r1->p,vec3d_Mul(r1->d,0.5f));
        vec3d m2 = vec3d_Add(r2.p, vec3d_Mul(r2.d, 0.5f));

        vec3d l = vec3d_Add(r1->d,r2.d);
        vec3d d = vec3d_Sub(m2,m1);
        d = (vec3d){ d.x / l.x,d.y / l.y,d.z / l.z,1.0f };

        if(F32_Abs(d.x)>F32_Abs(d.y)){
            if(F32_Abs(d.x)>F32_Abs(d.z)){
				if(d.x>0.0f){
                	m1.x = m2.x - l.x * 0.5f;
                	r1->p.x = m1.x - r1->d.x * 0.5f;

                    if(Funcs[0]) Funcs[0](Data);
            	}else{
            	    m1.x = m2.x + l.x * 0.5f;
            	    r1->p.x = m1.x - r1->d.x * 0.5f;

                    if(Funcs[1]) Funcs[1](Data);
            	}
			}else{
				if(d.z>0.0f){
                	m1.z = m2.z - l.z * 0.5f;
                	r1->p.z = m1.z - r1->d.z * 0.5f;

                    if(Funcs[2]) Funcs[2](Data);
            	}else{
            	    m1.z = m2.z + l.z * 0.5f;
            	    r1->p.z = m1.z - r1->d.z * 0.5f;

                    if(Funcs[3]) Funcs[3](Data);
            	}
			}
        }else{
            if(F32_Abs(d.y)>F32_Abs(d.z)){
				if(d.y>0.0f){
                	m1.y = m2.y - l.y * 0.5f;
                	r1->p.y = m1.y - r1->d.y * 0.5f;

                    if(Funcs[4]) Funcs[4](Data);
            	}else{
            	    m1.y = m2.y + l.y * 0.5f;
            	    r1->p.y = m1.y - r1->d.y * 0.5f;

                    if(Funcs[5]) Funcs[5](Data);
            	}
			}else{
				if(d.z>0.0f){
                	m1.z = m2.z - l.z * 0.5f;
                	r1->p.z = m1.z - r1->d.z * 0.5f;

                    if(Funcs[2]) Funcs[2](Data);
            	}else{
            	    m1.z = m2.z + l.z * 0.5f;
            	    r1->p.z = m1.z - r1->d.z * 0.5f;

                    if(Funcs[3]) Funcs[3](Data);
            	}
			}
        }
    }
}

#endif