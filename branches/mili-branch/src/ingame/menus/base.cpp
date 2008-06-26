
#include "base.h"
#include <typeinfo>
#include "../../TA3D_Exception.h"

using namespace TA3D::Exceptions;

namespace TA3D
{
namespace Menus
{

    Abstract::Abstract()
    {
        pTypeName = typeid(*this).name();
    }

    bool Abstract::execute()
    {
        // Executing if initializing succeeded
        bool r = (doGuardInitialize() && doGuardExecute());
        // Finalizing
        doGuardFinalize();
        return (r);
    }

        
    bool Abstract::doGuardInitialize()
    {
        GuardEnter(pTypeName + ".doInitialize()");
        bool r = doInitialize();
        GuardLeave();
        return r;
    }

    bool Abstract::doGuardExecute()
    {
        GuardEnter(pTypeName + ".doExecute()");
        bool r = doExecute();
        GuardLeave();
        return r;
    }

    void Abstract::doGuardFinalize()
    {
        GuardEnter(pTypeName + ".doFinalize()");
        doFinalize();
        GuardLeave();
    }

} // namespace Menus
} // namespace TA3D
