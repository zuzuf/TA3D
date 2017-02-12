
#include <yuni/yuni.h>
#include <yuni/extra/markdown/reader.h>
#include <yuni/extra/markdown/renderer/html.h>
#include <yuni/core/io/file.h>


int main(int argc, char** argv)
{
	if (argc < 2)
		return 0;

	using namespace Yuni;

	Markdown::Reader  reader;
	Markdown::Renderer::Html html;
	CustomString<4096> buffer;

	for (int i = 1; i < argc; ++i)
	{
		// filename
		const char* const filename = argv[i];

		Core::IO::File::Stream file;
		// opening out file
		if (file.open(filename))
		{
			reader.beginDocument(argv[i]);
			// A buffer. The given capacity will be the maximum length for a single line
			while (file.readline(buffer))
				reader += buffer;
		
			reader.endDocument();

			html.render(reader);
		}
	}

	return 0;
}


