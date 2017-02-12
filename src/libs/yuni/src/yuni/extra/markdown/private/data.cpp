
#include "data.h"
#include "../node.h"
#include "../fwd.h"
#include "../../../core/math.h"


namespace Yuni
{
namespace Private
{
namespace Markdown
{


	void ReaderData::reset()
	{
		filename.clear();
		line   = defaultLinePosition;
		column = defaultColumnPosition;
		pQuoteLevel = 0;

		pLine.clear();
		if (pLine.capacity() > 4096)
			pLine.shrink();

		pMapID.clear();
		pLastSignature.clear();
		pStackSize = 2;

		// creating the root node
		Node::Ptr newDocument = new Node(Node::document);
		Node::Ptr newBody     = new Node(Node::body);
		pTOC                  = new Node(Node::toc);
		Node::Ptr newHead     = new Node(Node::head);
		*newBody += pTOC;
		*newDocument += newHead;
		*newDocument += newBody;
		pStack[0] = newDocument;
		pStack[1] = newBody;
	}


	void ReaderData::parse(const StringAdapter& text)
	{
		// readline
		StringAdapter::Size offset = 0;
		do
		{
			const StringAdapter::Size p = text.find('\n', offset);
			if (p == StringAdapter::npos)
			{
				// The EOL has not been found, keeping the piece of string for later
				pLine.append(text, text.size() - offset, offset);
				return;
			}

			pLine.append(text, p - offset, offset);
			parseLine();

			// go to the next line
			++line;
			pLine.clear();
			offset = p + 1;
		}
		while (true);
	}


	void ReaderData::flush()
	{
		if (pLine.notEmpty())
			parseLine();

		// clear the map ID
		pMapID.clear();
		// releasing the stack
		// (however the first node must not be released yet)
		for (unsigned int i = 1; i != stackLimit; ++i)
			pStack[i] = nullptr;

		postParseTree(pStack[0]);

		if (pTOC->empty())
		{
			pTOC->detachFromParent();
			pTOC = nullptr;
		}
		// printing the root node
		//pStack[0]->dump(std::cout);
	}


	void ReaderData::parseLine()
	{
		// removing any final and unwanted \r
		if (pLine.notEmpty() && pLine.last() == '\r')
			pLine.removeLast();

		// Trimming the right side
		// Determine in the same time if a line break is present (at least
		// two spaces)
		{
			const unsigned int oldSize = pLine.size();
			pLine.trimRight(" \t");
			pHasLineBreak = pLine.size() + 1 < oldSize;
		}

		// Getting the signature of the current line
		// This signature will help us to build the parse tree
		// In the same time, the position of the first usefull character will be
		// retrieved (inner text)
		const unsigned int x = signatureForTheCurrentLine();

		// According the final type of node, we may have to add a paragraph node
		if (Node::ShouldAppendAParagraphNode(pCurrentLineSignature.lastType()))
			pCurrentLineSignature.add(Node::paragraph, pCurrentLineSignature.lastOffset());

		// The current line may refer to a pragma command
		// Try to find it and to execute it
		if (executePragma())
		{
			// A pragma command has been found. The current line must be ignored
			return;
		}

		// No pragma command. We can go on !
		if (debug)
		{
			CustomString<100,false> strline = line;
			strline << ", @=" << x << ' ';
			if (strline.size() < 20)
				strline.resize(20, '.');
			strline += ' ';
			std::cout << "[markdown:debug] " << filename << ":" << strline;
			pCurrentLineSignature.print(std::cout);
		}

		// Updating the stack accordingly (based on the current and the last signature)
		updateStack();

		// text
		{
			StringAdapter text;
			if (x < pLine.size())
			{
				text.adapt(pLine.c_str() + x, pLine.size() - x);
				text.trimLeft(" \t");
			}

			appendParagraphText(text);
		}

		// We have done with the current line
		// Keeping some data for the next one
		pLastSignature = pCurrentLineSignature;
	}


	void ReaderData::appendParagraphText(const StringAdapter& text)
	{
		assert(pStackSize > 1);

		// Reference to the last node
		const Node::Ptr& lastNode = pStack[pStackSize - 1];

		if (!text)
		{
			// Empty line. We may have to start a new paragraph
			// (only if innerText is empty however - to reduce useless memory allocation)
			if (pCurrentLineSignature.lastType() == Node::paragraph && !lastNode->empty())
				pushNewNode();
		}
		else
		{
			// At this point, we may have to promote the node to a header
			const char c = text.first();
			if ((c == '-' || c == '=') && String::npos == text.find_first_not_of(c, 1))
			{
				if (lastNode->type == Node::paragraph)
				{
					if (lastNode->size() == 1)
					{
						// Simple promotion. No need to node manipulation
						lastNode->type = (c == '-') ? Node::header2 : Node::header1;
					}
					else
					{
						Node::Ptr lastChild = lastNode->lastChild();
						if (lastChild->type == Node::text)
						{
							// Promotion to header ! Moving this child to the parent of the
							// last node
							const Node::Ptr& beforeLastNode = pStack[pStackSize - 2];
							assert(!(!beforeLastNode));
							Node::Ptr newParagraph = new Node(Node::paragraph);
							*beforeLastNode += newParagraph;
							lastChild->type = (c == '-') ? Node::header2 : Node::header1;
							lastChild->parent(newParagraph);
						}
					}
					// Adding a new node
					pushNewNode();
					return;
				}
				// TODO Throw a warning here
				return;
			}

			// Pushing the new text
			if (!lastNode->empty())
			{
				const Node::Ptr& sub = lastNode->lastChild();
				if (sub->type == Node::text)
				{
					String& innerText = sub->innerText;
					if (!innerText)
						innerText = text;
					else
						innerText << ' ' << text;
				}
				else
					*lastNode += new Node(Node::text, text);
			}
			else
				*lastNode += new Node(Node::text, text);

			// Adding a line break if asked
			if (pHasLineBreak)
			{
				if (debug)
					std::cout << "[markdown:debug] " << filename << ":" << line << "  added line break\n";
				*lastNode += new Node(Node::linebreak);
			}
		}
	}


	void ReaderData::pushNewNode()
	{
		assert(pStackSize > 1);

		const Node::Ptr& beforeLastNode = pStack[pStackSize - 2];
		assert(!(!beforeLastNode));
		Node::Ptr newParagraph = new Node(Node::paragraph);
		*beforeLastNode += newParagraph;
		pStack[pStackSize - 1] = newParagraph;
	}


	unsigned int ReaderData::signatureForTheCurrentLine()
	{
		// Signature for the current line
		Signature& signature = pCurrentLineSignature;
		signature.clear();

		// * > > *
		// **em**
		// * piko
		// *   sdfsdf
		// *   dsfsd
		for (unsigned int i = 0; i != pLine.size(); ++i)
		{
			switch (pLine[i])
			{
				case ' ':
					break;
				case '#':
					{
						unsigned int level = 1;
						unsigned int oldOffset = i;
						++i;
						while (pLine[i] == '#')
						{
							++i;
							++level;
						}
						if (level > 6)
							level = 6;
						static const Node::Type types[] =
						{
							Node::header1, // unused
							Node::header1,
							Node::header2,
							Node::header3,
							Node::header4,
							Node::header5,
							Node::header6,
						};
						signature.add(types[level], oldOffset);
						return i;
					}
				case '+':
				case '-':
				case '*':
					{
						// The next ID
						const unsigned int next = i + 1;
						if (next < pLine.size() && String::IsSpace((char)pLine[next]))
						{
							// OK It should be a list
							if (signature.checkLast(Node::unorderedList, i - 1))
							{
								// we previously got a bad token, something like **word**
								// reverting
								signature.pop();
								return i - 1;
							}
							signature.add(Node::unorderedList, i);
							signature.add(Node::listItem, i, true);
							break;
						}
						return i;
					}
				case '>':
					{
						signature.add(Node::quote, i);
						break;
					}
				case '_':
					{
						// Checking for horizontal line
						if (pLine[i + 1] == '_' && pLine[i + 2] == '_' && pLine[i + 3] != '_')
						{
							signature.promote(Node::hzLine);
							return i + 3;
						}
						return i;
					}
				default:
					return i;
			}
		}
		return pLine.size();
	}



	bool ReaderData::executePragma()
	{
		// TODO implement pragma commands
		return false;
	}


	void ReaderData::updateStack()
	{
		// aliases to the signatures
		const Signature& old = pLastSignature;
		const Signature& cur = pCurrentLineSignature;
		if (old.size == 1 && cur.size == 1)
		{
			// nothing to do - empty
			return;
		}

		unsigned int oldI = 1; // starting from 1, the root node can be safely ignored
		unsigned int curI = 1;
		const unsigned int maxOffset = Math::Min<unsigned int>(old.size, cur.size);

		for (unsigned int i = 1; i < maxOffset; ++i)
		{
			if (cur.forcePush[curI])
				break;
			if (cur.nodes[curI] == old.nodes[oldI] && cur.offsets[curI] == old.offsets[oldI])
			{
				// Oh ! Exactly the sames !
				++oldI;
				++curI;
				continue;
			}
			// Maybe we are still in the same block
			const unsigned int newCurI = curI + 1;
			if (newCurI < cur.size)
			{
				if (cur.nodes[newCurI] == old.nodes[oldI] && cur.offsets[newCurI] == old.offsets[oldI])
				{
					++oldI;
					curI += 2;
					continue;
				}
			}
			break;
		}

		// pop the stack
		if (oldI < old.size)
		{
			unsigned int newStackSize = pStackSize;
			for (unsigned int i = oldI; i < pStackSize; ++i)
			{
				// alias to the current node
				Node::Ptr& node = pStack[i];

				// decrease the pointer to the stacl
				--newStackSize;

				if (debug)
				{
					std::cout << "[markdown:debug]     - " << Node::TypeToCString(node->type)
						<< "   (stack size: " << newStackSize << ")\n";
				}
				// our algorithm may keep some unsed paragraph nodes
				if (node->type == Node::paragraph && node->empty())
					(node->parent())->remove(node);
			}
			pStackSize = oldI;
		}

		// creating the new nodes
		if (curI < cur.size)
		{
			assert(pStackSize > 0 && "The root node must exist");

			for (unsigned int i = curI; i < cur.size; ++i)
			{
				if (debug)
				{
					std::cout << "[markdown:debug]     + " << Node::TypeToCString(static_cast<Node::Type>(cur.nodes[i]))
						<< "   (stack size: " << pStackSize <<  ")\n";
				}

				// Creating the new node
				Node::Ptr node = new Node(static_cast<Node::Type>(cur.nodes[i]));
				// Adding as a new child of its parent
				*(pStack[pStackSize - 1]) += node;
				// adding it to the stack
				pStack[pStackSize] = node;
				++pStackSize;
			}
		}
	}


	void ReaderData::appendTOCEntry(const Node& node, unsigned int level)
	{
		// Trying to find the location of the parent node for the entry
		Node::Ptr it = pTOC;
		do
		{
			if (!level)
			{
				if (it->empty())
				{
					Node::Ptr ol = new Node(Node::orderedList);
					*it += ol;
					it = ol;
				}
				else
				{
					it = it->lastChild();
					assert(!(!it));
					assert(it->type != Node::listItem);
				}
				break;
			}
			if (it->empty())
			{
				Node::Ptr ol = new Node(Node::orderedList);
				Node::Ptr li = new Node(Node::listItem);
				*ol += li;
				*it += ol;
				it = li;
			}
			else
			{
				it = it->lastChild();
				assert(!(!it));
				assert(it->type != Node::listItem);
				it = it->lastChild();
				assert(!(!it));
				assert(it->type == Node::listItem);
			}
			--level;
		}
		while (true);

		// A few last checks
		assert(!(!it));
		assert(it->type != Node::listItem);

		// Adding the new entry
		Node* li = new Node(Node::listItem);
		node.flattenText(li->innerText);
		*it += li;
	}


	void ReaderData::postParseTree(const Node::Ptr& node)
	{
		assert(!(!node));
		const Node& rawNode = *node;

		// Type of the node
		const Node::Type type = rawNode.type;
		assert(static_cast<unsigned int>(type) < static_cast<unsigned int>(Node::maxType));

		if (type <= Node::header6 && type >= Node::header1)
		{
			// This node must be referenced into the TOC
			// Determining its header level
			unsigned int level = static_cast<unsigned int>(type) - static_cast<unsigned int>(Node::header1);
			// Append a new entry into the TOC
			appendTOCEntry(rawNode, level);
		}
		else
		{
			// children
			if (!rawNode.empty())
			{
				const Node::iterator end = rawNode.end();
				for (Node::iterator i = rawNode.begin(); i != end; ++i)
					postParseTree(*i);
			}
		}
	}





} // namespace Markdown
} // namespace Private
} // namespace Yuni

