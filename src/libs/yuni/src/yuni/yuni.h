#ifndef __YUNI_H__
# define __YUNI_H__


# ifdef YUNI_CONFIG_H_LOCATION
#	include YUNI_CONFIG_H_LOCATION
# else
#	include "config.h"
# endif


/* Platform checks */
# include "platform.h"
/* Standard Preprocessor tools */
# include "core/preprocessor/std.h"
/* Operating System / Capabilities auto-detection */
# include "core/system/capabilities.h"
/* Standard Types */
# include "core/system/stdint.h"



# ifdef __cplusplus /* Only with a C++ Compiler */

/*!
** \brief The Yuni Framework
*/
namespace Yuni
{}

/* nullptr */
# include "core/system/nullptr.h"
/* All standard forward declarations */
# include "core/fwd.h"

# endif


#endif /* __YUNI_H__ */
