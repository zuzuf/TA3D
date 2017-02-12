
#include <vector>
#include <yuni/yuni.h>
#include <yuni/application/console.h>
#include <yuni/core/string.h>
#include <yuni/audio/queueservice.h>
#include <yuni/core/system/sleep.h>
#include <yuni/core/math.h>

using namespace Yuni;


class MovingSource : public Application::Console
{
public:
	MovingSource(int argc, char* argv[])
		:Application::Console(argc, argv),
		pFileName(argv[1])
	{
		pAudio.start();
	}

	virtual ~MovingSource()
	{
		pAudio.stop();
	}

	virtual void onExecute()
	{
		const float LIMIT = 20.0f;

		String emitterName("Emitter");
		if (!pAudio.emitter.add(emitterName))
		{
			std::cerr << "Emitter creation failed !" << std::endl;
			return;
		}
		pAudio.bank.load(pFileName);
		pAudio.emitter.attach(emitterName, pFileName);
		pAudio.emitter.play(emitterName);
		Point3D<> position;
		bool inverse = false;
		position.y = -LIMIT;

		pAudio.emitter.move(emitterName, position);

		for (unsigned int i = 0; i < 2000; ++i)
		{
			Yuni::SuspendMilliSeconds(100);

			position.y += 1.0f * (inverse ? -1.0f : 1.0f);

			if (!inverse && position.y > LIMIT)
				inverse = true;
			else
			{
				if (inverse && position.y < -LIMIT)
					inverse = false;
			}
			pAudio.emitter.move(emitterName, position);
		}
	}

private:
	String pFileName;
	Audio::QueueService pAudio;

}; // class MovingSource




int main(int argc, char* argv[])
{
	if (argc < 2)
		return false;

	// Yuni main loop
	MovingSource app(argc, argv);
	app.onExecute();
	return app.exitCode();
}

