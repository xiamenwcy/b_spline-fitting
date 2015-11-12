/*************************************************
* Copyright (c) 2014-至今,厦门大学计算机图形学实验室Graphics@XMU
* All rights reserved.
*
* 文件名称：gl.h 
* 摘要：主函数，入口函数
*
* 当前版本：2.0
* 作者：王财勇
* 完成日期：2015年11月10日
*
* 取代版本：1.9
* 原作者：王财勇
* 完成日期：2015年4月1日
*/

#ifndef GL_WRAPPERS_HH
#define GL_WRAPPERS_HH

#include <stdlib.h>
#include<gl/glut.h>
#include <OpenMesh/Core/Geometry/VectorT.hh>
//=============================================================================
namespace GL {
//=============================================================================


//-------------------------------------------------------------------- glVertex

/// Wrapper: glVertex for OpenMesh::Vec2i
inline void glVertex(const OpenMesh::Vec2i& _v)  { glVertex2i(_v[0], _v[1]); }
/// Wrapper: glVertex for OpenMesh::Vec2f
inline void glVertex(const OpenMesh::Vec2f& _v)  { glVertex2f(_v[0],_v[1]); }
/// Wrapper: glVertex for OpenMesh::Vec2d
inline void glVertex(const OpenMesh::Vec2d& _v)  { glVertex2d(_v[0],_v[1]); }

/// Wrapper: glVertex for OpenMesh::Vec3f
inline void glVertex(const OpenMesh::Vec3f& _v)  { glVertex3f(_v[0],_v[1],_v[2]); }
/// Wrapper: glVertex for OpenMesh::Vec3d
inline void glVertex(const OpenMesh::Vec3d& _v)  { glVertex3d(_v[0],_v[1],_v[2]); }

/// Wrapper: glVertex for OpenMesh::Vec4f
inline void glVertex(const OpenMesh::Vec4f& _v)  { glVertex4f(_v[0],_v[1],_v[2],_v[3]); }
/// Wrapper: glVertex for OpenMesh::Vec4d
inline void glVertex(const OpenMesh::Vec4d& _v)  { glVertex4d(_v[0],_v[1],_v[2],_v[3]); }



//------------------------------------------------------------------- glTexCoord

/// Wrapper: glTexCoord for 1D float
inline void glTexCoord(float _t) { glTexCoord1f(_t); }
/// Wrapper: glTexCoord for 1D double
inline void glTexCoord(double _t) { glTexCoord1d(_t); }

/// Wrapper: glTexCoord for OpenMesh::Vec2f
inline void glTexCoord(const OpenMesh::Vec2f& _t) { glTexCoord2f(_t[0],_t[1]); }
/// Wrapper: glTexCoord for OpenMesh::Vec2d
inline void glTexCoord(const OpenMesh::Vec2d& _t) { glTexCoord2d(_t[0],_t[1]); }

/// Wrapper: glTexCoord for OpenMesh::Vec3f
inline void glTexCoord(const OpenMesh::Vec3f& _t) { glTexCoord3f(_t[0],_t[1],_t[2]); }
/// Wrapper: glTexCoord for OpenMesh::Vec3d
inline void glTexCoord(const OpenMesh::Vec3d& _t) { glTexCoord3d(_t[0],_t[1],_t[2]); }

/// Wrapper: glTexCoord for OpenMesh::Vec4f
inline void glTexCoord(const OpenMesh::Vec4f& _t) { glTexCoord4f(_t[0],_t[1],_t[2],_t[3]); }
/// Wrapper: glTexCoord for OpenMesh::Vec4d
inline void glTexCoord(const OpenMesh::Vec4d& _t) { glTexCoord4d(_t[0],_t[1],_t[2],_t[3]); }



//--------------------------------------------------------------------- glNormal

/// Wrapper: glNormal for OpenMesh::Vec3f
inline void glNormal(const OpenMesh::Vec3f& _n)  { glNormal3f(_n[0],_n[1],_n[2]); }
/// Wrapper: glNormal for OpenMesh::Vec3d
inline void glNormal(const OpenMesh::Vec3d& _n)  { glNormal3d(_n[0],_n[1],_n[2]); }



//---------------------------------------------------------------------- glColor

/// Wrapper: glColor for OpenMesh::Vec3f
inline void glColor(const OpenMesh::Vec3f&  _v)  { glColor3f(_v[0],_v[1],_v[2]); }
/// Wrapper: glColor for OpenMesh::Vec3uc
inline void glColor(const OpenMesh::Vec3uc& _v)  { glColor3ub(_v[0],_v[1],_v[2]); }

/// Wrapper: glColor for OpenMesh::Vec4f
inline void glColor(const OpenMesh::Vec4f&  _v)  { glColor4f(_v[0],_v[1],_v[2],_v[3]); }
/// Wrapper: glColor for OpenMesh::Vec4uc
inline void glColor(const OpenMesh::Vec4uc&  _v) { glColor4ub(_v[0],_v[1],_v[2],_v[3]); }



//-------------------------------------------------------------- glVertexPointer

/// Wrapper: glVertexPointer for OpenMesh::Vec2f
inline void glVertexPointer(const OpenMesh::Vec2f* _p) 
{ ::glVertexPointer(2, GL_FLOAT, 0, _p); }
/// Wrapper: glVertexPointer for OpenMesh::Vec2d
inline void glVertexPointer(const OpenMesh::Vec2d* _p) 
{ ::glVertexPointer(2, GL_DOUBLE, 0, _p); }

/// Wrapper: glVertexPointer for OpenMesh::Vec3f
inline void glVertexPointer(const OpenMesh::Vec3f* _p) 
{ ::glVertexPointer(3, GL_FLOAT, 0, _p); }
/// Wrapper: glVertexPointer for OpenMesh::Vec3d
inline void glVertexPointer(const OpenMesh::Vec3d* _p) 
{ ::glVertexPointer(3, GL_DOUBLE, 0, _p); }

/// Wrapper: glVertexPointer for OpenMesh::Vec4f
inline void glVertexPointer(const OpenMesh::Vec4f* _p) 
{ ::glVertexPointer(4, GL_FLOAT, 0, _p); }
/// Wrapper: glVertexPointer for OpenMesh::Vec4d
inline void glVertexPointer(const OpenMesh::Vec4d* _p) 
{ ::glVertexPointer(4, GL_DOUBLE, 0, _p); }

/// original method
inline void glVertexPointer(GLint n, GLenum t, GLsizei s, const GLvoid *p)
{ ::glVertexPointer(n, t, s, p); }



//-------------------------------------------------------------- glNormalPointer

/// Wrapper: glNormalPointer for OpenMesh::Vec3f
inline void glNormalPointer(const OpenMesh::Vec3f* _p)
{ ::glNormalPointer(GL_FLOAT, 0, _p); }
/// Wrapper: glNormalPointer for OpenMesh::Vec3d
inline void glNormalPointer(const OpenMesh::Vec3d* _p)
{ ::glNormalPointer(GL_DOUBLE, 0, _p); }

/// original method
inline void glNormalPointer(GLenum t, GLsizei s, const GLvoid *p)
{ ::glNormalPointer(t, s, p); }



//--------------------------------------------------------------- glColorPointer

/// Wrapper: glColorPointer for OpenMesh::Vec3uc
inline void glColorPointer(const OpenMesh::Vec3uc* _p)
{ ::glColorPointer(3, GL_UNSIGNED_BYTE, 0, _p); }
/// Wrapper: glColorPointer for OpenMesh::Vec3f
inline void glColorPointer(const OpenMesh::Vec3f* _p)
{ ::glColorPointer(3, GL_FLOAT, 0, _p); }

/// Wrapper: glColorPointer for OpenMesh::Vec4uc
inline void glColorPointer(const OpenMesh::Vec4uc* _p)
{ ::glColorPointer(4, GL_UNSIGNED_BYTE, 0, _p); }
/// Wrapper: glColorPointer for OpenMesh::Vec4f
inline void glColorPointer(const OpenMesh::Vec4f* _p)
{ ::glColorPointer(4, GL_FLOAT, 0, _p); }

/// original method
inline void glColorPointer(GLint n, GLenum t, GLsizei s, const GLvoid *p)
{ ::glColorPointer(n, t, s, p); }



//------------------------------------------------------------ glTexCoordPointer

/// Wrapper: glTexCoordPointer for float
inline void glTexCoordPointer(const float* _p) 
{ ::glTexCoordPointer(1, GL_FLOAT, 0, _p); }
/// Wrapper: glTexCoordPointer for OpenMesh::Vec2d
inline void glTexCoordPointer(const double* _p) 
{ ::glTexCoordPointer(1, GL_DOUBLE, 0, _p); }

/// Wrapper: glTexCoordPointer for OpenMesh::Vec2f
inline void glTexCoordPointer(const OpenMesh::Vec2f* _p) 
{ ::glTexCoordPointer(2, GL_FLOAT, 0, _p); }
/// Wrapper: glTexCoordPointer for OpenMesh::Vec2d
inline void glTexCoordPointer(const OpenMesh::Vec2d* _p) 
{ ::glTexCoordPointer(2, GL_DOUBLE, 0, _p); }

/// Wrapper: glTexCoordPointer for OpenMesh::Vec3f
inline void glTexCoordPointer(const OpenMesh::Vec3f* _p) 
{ ::glTexCoordPointer(3, GL_FLOAT, 0, _p); }
/// Wrapper: glTexCoordPointer for OpenMesh::Vec3d
inline void glTexCoordPointer(const OpenMesh::Vec3d* _p) 
{ ::glTexCoordPointer(3, GL_DOUBLE, 0, _p); }

/// Wrapper: glTexCoordPointer for OpenMesh::Vec4f
inline void glTexCoordPointer(const OpenMesh::Vec4f* _p) 
{ ::glTexCoordPointer(4, GL_FLOAT, 0, _p); }
/// Wrapper: glTexCoordPointer for OpenMesh::Vec4d
inline void glTexCoordPointer(const OpenMesh::Vec4d* _p) 
{ ::glTexCoordPointer(4, GL_DOUBLE, 0, _p); }

/// original method
inline void glTexCoordPointer(GLint n, GLenum t, GLsizei s, const GLvoid *p)
{ ::glTexCoordPointer(n, t, s, p); }



//-----------------------------------------------------------------------------


/** Nice wrapper that outputs all current OpenGL errors to std::cerr.
    If no error is present nothing is printed.
**/
inline void glCheckErrors()
{
  GLenum error;
  while ((error = glGetError()) != GL_NO_ERROR)
  {
    std::cerr << "GL error: " << gluErrorString(error) << std::endl;
  }
}


//=============================================================================
}
//=============================================================================
#endif // GL_WRAPPERS_HH defined
//=============================================================================
