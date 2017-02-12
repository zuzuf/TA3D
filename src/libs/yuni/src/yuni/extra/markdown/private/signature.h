#ifndef __YUNI_EXTRA_MARKDOWN_SIGNATURE_H__
# define __YUNI_EXTRA_MARKDOWN_SIGNATURE_H__

# include "../node.h"
# include "../fwd.h"


namespace Yuni
{
namespace Private
{
namespace Markdown
{



	class Signature
	{
	public:
		//! Node
		typedef Yuni::Markdown::Node  Node;
		enum
		{
			hardLimit = Yuni::Markdown::stackLimit,
		};
	public:
		Signature()
			:size(0)
		{}

		Signature(const Signature& rhs)
			:size(rhs.size)
		{
			for (unsigned int i = 0; i != size; ++i)
			{
				nodes[i]   = rhs.nodes[i];
				offsets[i] = rhs.offsets[i];
			}
		}


		void clear()
		{
			size       = 2;
			offsets[0] = 0;
			nodes[0]   = Node::document;
			offsets[1] = 0;
			nodes[1]   = Node::body;

		}

		void add(const Node::Type node, unsigned int offset, bool force = false)
		{
			assert(size != hardLimit - 1 && "Two many indentation levels");
			assert(size != 0 && "A signature must at least contain the document node");

			nodes[size]     = node;
			offsets[size]   = offset;
			forcePush[size] = force;
			++size;
		}

		void promote(const Node::Type node)
		{
			assert(size > 1);
			nodes[size - 1] = node;
		}

		void pop()
		{
			assert(size > 1);
			--size;
		}

		bool checkLast(const Node::Type node, unsigned int offset) const
		{
			assert(size != 0);
			const unsigned int i = size - 1;
			return (nodes[i] == node && offsets[i] == offset);
		}


		inline Node::Type lastType() const
		{
			assert(size != 0);
			return static_cast<Node::Type>(nodes[size - 1]);
		}


		inline unsigned int lastOffset() const
		{
			assert(size != 0);
			return offsets[size - 1];
		}


		template<class StreamT> void print(StreamT& stream) const
		{
			for (unsigned int i = 0; i != size; ++i)
			{
				stream << '[' << Node::TypeToCString(static_cast<Node::Type>(nodes[i])) << ':'
					<< offsets[i] << ']';
			}
			stream << '\n';
		}


		Signature& operator = (const Signature& rhs)
		{
			size = rhs.size;
			for (unsigned int i = 0; i != size; ++i)
			{
				nodes[i]   = rhs.nodes[i];
				offsets[i] = rhs.offsets[i];
			}
			return *this;
		}

	public:
		unsigned int size;
		char nodes[hardLimit];
		unsigned int offsets[hardLimit];
		bool forcePush[hardLimit];

	}; // class Signature





} // namespace Markdown
} // namespace Private
} // namespace Yuni

#endif // __YUNI_EXTRA_MARKDOWN_SIGNATURE_H__
