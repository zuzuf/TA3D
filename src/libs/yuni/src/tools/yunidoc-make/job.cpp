
#include "job.h"
#include "logs.h"
#include <yuni/core/io/io.h>


#define SEP  Core::IO::Separator

using namespace Yuni;

Yuni::Job::QueueService<>  queueService;




CompileJob::CompileJob(const Yuni::String& input, const Yuni::String& htdocs)
	:pInput(input),
	pHtdocs(htdocs)
{
}


CompileJob::~CompileJob()
{
}


void CompileJob::onExecute()
{
	String target;
	String relative;
	const String::Vector::const_iterator end = pSources.end();
	for (String::Vector::const_iterator i = pSources.begin(); i != end; ++i)
	{
		const String& entry = *i;
		relative.assign(entry, entry.size() - pInput.size() - 1 - 10 - 1, pInput.size() + 1);
		target << pHtdocs << SEP << relative << SEP << "index.html";

		logs.info() << "building " << relative << SEP << "index.text";

	}
	/*
	   String relative;
	   relative.assign(pSourceFilename, pSourceFilename.size() - pInput.size() - 1 - 10 - 1, pInput.size() + 1);
	   String target;
	   target << pHtdocs << SEP << relative << SEP << "index.html";

	   logs.info() << "building " << relative << SEP << "index.text";
	   logs.debug() << "  :: source " << pSourceFilename;
	   logs.debug() << "  :: target " << target;
	 */
}


