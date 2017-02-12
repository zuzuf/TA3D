#ifndef __YUNI_CORE_IO_DIRECTORY_INFO_ITERATOR_H__
# define __YUNI_CORE_IO_DIRECTORY_INFO_ITERATOR_H__


// !!! Do not use includes here

public:

	struct Model
	{

		// Forward declarations
		class NullIterator;
		template<unsigned int FlagsT> class Iterator;


		class NullIterator
		{
		public:
			//! The type of the orignal object
			typedef String value_type;
			//! An unsigned integral type
			typedef uint64  uint64ype;
			//! A signed integral type
			typedef sint64  difference_type;

			enum
			{
				//! A non-zero value if the iterator can go forward
				canGoForward  = true,
				//! A non-zero value if the iterator can go backward
				canGoBackward = false,
			};


		public:

		protected:
			NullIterator()
			{}

			NullIterator(const NullIterator&)
			{}

			template<class ModelT>
			NullIterator(const ModelT&)
			{}

			static void forward(difference_type)
			{
				// Do nothing
			}

			template<class ModelT>
			static void reset(const ModelT&)
			{
				// Do nothing
			}

			bool equals(const NullIterator&)
			{
				return true;
			}

			template<class ModelT>
			bool equals(const ModelT& model)
			{
				return (model.pData == NULL);
			}

			template<class ModelT>
			difference_type distance(const ModelT&) const
			{
				return 0;
			}

			// ::Char& operator * ()

			// ::Char* operator -> ()

		}; // class NullIterator



		/*!
		** \brief
		**
		** \tparam FlagsT See enum Yuni::Core::IO::Directory::Info::OptionIterator
		** \see enum Yuni::Core::IO::Directory::Info::OptionIterator
		*/
		template<unsigned int FlagsT>
		class Iterator
		{
		public:
			//! The type of the orignal object
			typedef String value_type;
			//! An unsigned integral type
			typedef uint64 uint64ype;
			//! A signed integral type
			typedef sint64 difference_type;

			enum
			{
				//! A non-zero value if the iterator can go forward
				canGoForward  = true,
				//! A non-zero value if the iterator can go backward
				canGoBackward = false,
			};

		public:
			bool isFile() const
			{
				return Private::Core::IO::Directory::IteratorDataIsFile(pData);
			}

			bool isFolder() const
			{
				return Private::Core::IO::Directory::IteratorDataIsFolder(pData);
			}

			const String& filename() const
			{
				return Private::Core::IO::Directory::IteratorDataFilename(pData);
			}

			const String& parent() const
			{
				return Private::Core::IO::Directory::IteratorDataParentName(pData);
			}

			uint64 size() const
			{
				return Private::Core::IO::Directory::IteratorDataSize(pData);
			}

			bool valid() const
			{
				return pData != NULL;
			}

			bool operator ! () const
			{
				return !pData;
			}

		protected:
			Iterator()
				:pData(NULL)
			{}
			template<class StringT> explicit Iterator(const StringT& directory)
			{
				YUNI_STATIC_ASSERT(Traits::CString<StringT>::valid, InvalidTypeForBuffer);
				YUNI_STATIC_ASSERT(Traits::Length<StringT>::valid,  InvalidTypeForBufferSize);

				// Initializing
				pData = Private::Core::IO::Directory::IteratorDataCreate(
					Traits::CString<StringT>::Perform(directory), // c-string
					Traits::Length<StringT>::Value(directory),    // length of the string
					FlagsT);                                      // flags
				// We must forward once to get the first item
				forward();
			}

			Iterator(const NullIterator&)
				:pData(NULL)
			{}

			Iterator(const Iterator& copy)
				:pData(Private::Core::IO::Directory::IteratorDataCopy(copy.pData))
			{}
			~Iterator()
			{
				if (pData)
					Private::Core::IO::Directory::IteratorDataFree(pData);
			}

			void forward()
			{
				pData = Private::Core::IO::Directory::IteratorDataNext(pData);
			}

			void forward(difference_type n)
			{
				while (n > 0)
				{
					forward();
					--n;
				}
			}

			template<class ModelT>
			void reset(const ModelT& model)
			{
				if (pData)
					Private::Core::IO::Directory::IteratorDataFree(pData);
				pData = Private::Core::IO::Directory::IteratorDataCopy(model.pData);
			}

			template<class ModelT>
			difference_type distance(const ModelT&) const
			{
				return 0;
			}

			bool equals(const NullIterator&) const
			{
				return (pData == NULL);
			}

			template<class ModelT>
			bool equals(const ModelT& model) const
			{
				return (!pData && !model.pData);
			}

			const String& operator * ()
			{
				assert(pData != NULL);
				return Private::Core::IO::Directory::IteratorDataName(pData);
			}

			const String* operator -> ()
			{
				assert(pData != NULL);
				return &Private::Core::IO::Directory::IteratorDataName(pData);
			}

		private:
			//! Platform-dependant data implementation
			Private::Core::IO::Directory::IteratorData* pData;

		}; // class Iterator



	}; // class Model



#endif // __YUNI_CORE_IO_DIRECTORY_INFO_ITERATOR_H__
