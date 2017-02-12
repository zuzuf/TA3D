
#include <yuni/yuni.h>
#include <yuni/core/event.h>
#include <yuni/core/string.h>
#include <iostream>


using namespace Yuni;


// A global event
//
// Here is an event with 2 arguments :
Event<void (const String&, const String&)> emailHasBeenReceived;




/*!
** \brief A subscriber class
**
** To be able to receive events, the class must inherit from the base class
** `Yuni::Event::Receiver`
*/
class Subscriber : public IEventObserver<Subscriber>
{
public:
	//! Constructor
	Subscriber()
	{
		// This class wants to be aware when an event is fired
		emailHasBeenReceived.connect(this, &Subscriber::onMailReceived);
	}

	//! Destructor
	virtual ~Subscriber()
	{
		// It is really encouraged to disconnect all events as soon as possible.
		// The base class will do it, however the inherited class will already
		// be removed from the vtable and it might lead to a critical error
		destroyBoundEvents();
	}

	/*!
	** \brief Event callback
	**
	** This method will be called each time the event `emailHasBeenReceived`
	** is fired
	*/
	void onMailReceived(const String& subject, const String& from)
	{
		std::cout << "a mail has been received :" << std::endl
			<< "\tFrom    : " << from << std::endl
			<< "\tSubject : " << subject << std::endl;
	}
};



int main(void)
{
	// A subscriber, able to receive some events
	Subscriber sc;
	// A new event
	emailHasBeenReceived("My great subject !", "paul@utopia.org");

	return 0;
}

