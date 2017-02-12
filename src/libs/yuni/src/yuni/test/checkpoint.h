#ifndef __YUNI_TEST_CHECKPOINT_H__
# define __YUNI_TEST_CHECKPOINT_H__


# define YUNI_TEST_ID_MAXLENGTH 255


namespace Yuni
{
namespace Test
{


	class Checkpoint
	{
	public:
		//! \name Constructor & Destructor
		//@{
		/*!
		** \brief Default Constructor
		*/
		explicit Checkpoint(const char* id);
		//! Destructor
		~Checkpoint();
		//@}

		bool operator () (bool b);
		bool operator () (bool b, const char* msg);

	private:
		Checkpoint() {}

	private:
		//! ID
		char pID[YUNI_TEST_ID_MAXLENGTH];
		//! Result
		bool pResult;

	}; // class Checkpoint





	class TimedCheckpoint
	{
	public:
		//! \name Constructor & Destructor
		//@{
		/*!
		** \brief Default Constructor
		*/
		explicit TimedCheckpoint(const char* id);
		//! Destructor
		~TimedCheckpoint();
		//@}

		bool operator () (bool b);
		bool operator () (bool b, const char* msg);

	private:
		TimedCheckpoint() {}

	private:
		//! ID
		char pID[YUNI_TEST_ID_MAXLENGTH];
		//! Start time
		unsigned int pStartTime;

		//! Result
		bool pResult;

	}; // class Checkpoint





} // namespace Test
} // namespace Yuni

#endif // __YUNI_TEST_CHECKPOINT_H__
