#ifndef __YUNI_DOCMAKE_JOB_H__
# define __YUNI_DOCMAKE_JOB_H__

# include <yuni/yuni.h>
# include <yuni/core/string.h>
# include <yuni/job/job.h>
# include <yuni/job/queue.h>
# include <vector>



class CompileJob : public Yuni::Job::IJob
{
public:
	CompileJob(const Yuni::String& input, const Yuni::String& htdocs);
	virtual ~CompileJob();

	void add(const Yuni::String& filename) {pSources.push_back(filename);}

private:
	virtual void onExecute();

private:
	const Yuni::String& pInput;
	const Yuni::String& pHtdocs;
	Yuni::String::Vector  pSources;
};


extern Yuni::Job::QueueService<>  queueService;


#endif // __YUNI_DOCMAKE_JOB_H__
