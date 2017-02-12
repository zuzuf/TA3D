#ifndef __YUNI_CORE_MATH_GEOMETRY_VERTEX_H__
# define __YUNI_CORE_MATH_GEOMETRY_VERTEX_H__

# include "../../../yuni.h"
# include <vector>
# include "point3D.h"
# include "vector3D.h"
# include "../../string.h"
# include "../../smartptr.h"



namespace Yuni
{

	// Forward declaration
	class Edge;



	/*!
	** \brief A vertex is a point in space which is the start of two edges of a triangle in a mesh.
	** \ingroup Gfx
	*/
	class Vertex
	{
	public:
		//! \name Constructors and destructors
		//@{
		/*!
		** \brief Default constructor
		*/
		Vertex(): pPosition()
		{}
		Vertex(const Point3D<float>& position): pPosition(position)
		{}
		template<typename U, typename V, typename W>
		Vertex(const U x, const V y, const W z): pPosition(x, y, z)
		{}
		~Vertex() {}
		//@}


		/*!
		** \brief Position of the vertex in space
		*/
		const Point3D<float>& position() const {return pPosition;}


		/*!
		** \brief Get a string representing the object to use as hash key
		*/
		String toString() const
		{ return String() << pPosition.x << "," << pPosition.y << "," << pPosition.z; }

		//! \name Operators
		//@{

		/*!
		** \brief Comparison operator (equal with)
		**
		** \param other The other vertex to compare with
		** \return True if the two vertices are coincident
		*/
		bool operator == (const Vertex& other) const {return pPosition == other.position();}

		/*!
		** \brief Comparison operator (non equal with)
		**
		** \param other The other vertex to compare with
		** \return True if the two vertices are distinct
		*/
		bool operator != (const Vertex& other) const {return !(*this == other);}

		//@}

		/*!
		** \brief Generate the OpenGL code for this vertex
		**
		** \param[in,out] out An output stream
		** \return The output stream `out`
		*/
		std::ostream& glPrint(std::ostream& out) const
		{
			out << "glVertex3f" << pPosition << ";" << std::endl;
			return out;
		}


	private:
		//! List of edges
		typedef std::vector< SmartPtr<Edge> > EdgeList;

	private:
		//! Edges linked to this vertex
		EdgeList pEdges;
		//! Position of the vertex in space
		Point3D<float> pPosition;
		//! Normal of the vertex (useful for lighting)
		Vector3D<float> pNormal;

	}; // Vertex




} // namespace Yuni

#endif // __YUNI_CORE_MATH_GEOMETRY_VERTEX_H__
