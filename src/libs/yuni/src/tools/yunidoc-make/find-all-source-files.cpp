
#include "make.h"
#include <yuni/core/io/directory/info.h>
#include "job.h"
#include "logs.h"


using namespace Yuni;


void Make::findAllSourceFiles()
{
	std::vector<CompileJob*> jobs;
	jobs.resize(nbJobs);
	for (unsigned int i = 0; i != jobs.size(); ++i)
		jobs[i] = new CompileJob(input, htdocs);

	Core::IO::Directory::Info info(input);
	String relative;
	unsigned int slotIndex = 0;
	unsigned int count = 0;
	for (Core::IO::Directory::Info::recursive_iterator i = info.recursive_begin(); i != info.recursive_end(); ++i)
	{
		// assert
		assert(!(!(*i)));
		// alias
		const String& name = *i;
		const String& filename = i.filename();

		if (!i.isFile() || name != "index.text")
			continue;
		assert(filename.size() > input.size());
		assert(slotIndex < jobs.size());

		jobs[slotIndex]->add(filename);
		if (++slotIndex >= jobs.size())
			slotIndex = 0;
		++count;
	}

	if (count)
	{
		logs.info() << count << (count > 1 ? " pages to build, " : "page to build, ") << nbJobs << (nbJobs > 1 ? " threads" : " thread");
		for (unsigned int i = 0; i != jobs.size(); ++i)
			queueService += jobs[i];
	}
	else
	{
		for (unsigned int i = 0; i != jobs.size(); ++i)
			delete jobs[i];
	}
}


