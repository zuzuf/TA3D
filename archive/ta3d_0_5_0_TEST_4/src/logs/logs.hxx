#ifndef __XX_LIB_LOGS_HXX__
# define __XX_LIB_LOGS_HXX__


# define XX_LIB_LOGS_IMPLEMENT_OPERATOR(LogMsgClass) \
    template <> inline \
    Log& Log::operator << (LogMsgClass l) \
    { \
      if (l.minimalLevel() <= level && pOut) \
      { \
        (*pOut)  << l.date() \
          << (pOut == &std::cout || pOut == &std::cerr ? l.color() : "") \
          << l.header() \
          << (pOut == &std::cout || pOut == &std::cerr ? l.resetColor() : "") \
          << str() \
          << std::endl; \
      } \
      if (l.minimalLevel() <= level && pCallback) \
      { \
        pCallback(l.header().c_str(), str().c_str()); \
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
