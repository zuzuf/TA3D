#ifndef __XX_LIB_LOGS_HXX__
# define __XX_LIB_LOGS_HXX__


# define XX_LIB_LOGS_IMPLEMENT_OPERATOR(LogMsgClass) \
    template <> inline \
    Log& Log::operator << (LogMsgClass l) \
    { \
      if (l.minimalLevel() <= level) \
      { \
        String msg(str()); \
        if (pOut) \
        { \
            (*pOut) << l.date() << l.header() << msg << std::endl; \
            pOut->flush(); \
        } \
        std::cout << l.date() << l.color() << l.header() << l.resetColor() << msg << std::endl; \
        l.forwardToConsole(msg); \
        if (pCallback) \
            pCallback(l.header().c_str(), msg.c_str()); \
      } \
      \
      *((std::ostringstream*)this) << std::endl; \
      str(""); \
      clear(); \
      unlock(); \
      return (*this); \
    }


    XX_LIB_LOGS_IMPLEMENT_OPERATOR(LogDebugMsg)
    XX_LIB_LOGS_IMPLEMENT_OPERATOR(LogInfoMsg)
    XX_LIB_LOGS_IMPLEMENT_OPERATOR(LogWarningMsg)
    XX_LIB_LOGS_IMPLEMENT_OPERATOR(LogErrorMsg)
    XX_LIB_LOGS_IMPLEMENT_OPERATOR(LogCriticalMsg)

#undef XX_LIB_LOGS_IMPLEMENT_OPERATOR


      
#endif // __XX_LIB_LOGS_HXX__
