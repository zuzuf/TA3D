#include "math.h"
#include <time.h>
#include "../logs/logs.h"


//! The size of the random table
# define TA3D_RANDOM_TABLE_SIZE  0x100000
//! The mask used to get the next position in the random
# define TA3D_RANDOM_TABLE_MASK  0xFFFFF



namespace TA3D
{
namespace Math
{

    namespace
    {
        //! \name Random table
        //@{
        
        //! Current position in the random table
        int gRandomTablePosition = 0;

        //! The Random table
        uint32 gRandomTableData[TA3D_RANDOM_TABLE_SIZE];

        //@}
    }


    void InitializeRandomTable()
    {
        LOG_DEBUG("Initializing the random table...");
        // Rebuil the table
        srand(time(NULL));
        for (int i = 0; i < TA3D_RANDOM_TABLE_SIZE; ++i)
		    gRandomTableData[i] = TA3D_RAND();
        // Reset the position in this table
        gRandomTablePosition = 0;
        LOG_DEBUG("Initializing the random table: Done.");
    }

    uint32 RandFromTable()
    {
        // The next position in the table
        gRandomTablePosition = (gRandomTablePosition + 1) & TA3D_RANDOM_TABLE_MASK;
        // The random value
        return gRandomTableData[gRandomTablePosition];
    }



} // namespace Math
} // namespace TA3D
