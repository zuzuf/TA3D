#include "../../stdafx.h"
#include "skin.manager.h"

namespace TA3D
{
    SKIN_MANAGER skin_manager;

    SKIN_MANAGER::SKIN_MANAGER()
    {
        init();
    }

    SKIN_MANAGER::~SKIN_MANAGER()
    {
        destroy();
    }

    void SKIN_MANAGER::init()
    {
        skins.clear();
        hash_skin.emptyHashTable();
        hash_skin.initTable(__DEFAULT_HASH_TABLE_SIZE);
    }

    void SKIN_MANAGER::destroy()
    {
        for(std::vector<SKIN*>::iterator it = skins.begin() ; it != skins.end() ; ++it)
            delete *it;
        init();
    }

    SKIN *SKIN_MANAGER::load(const String& filename, const float scale)
    {
        String key = filename + format("-%.2f", scale);
        SKIN *pSkin = hash_skin.find(key);
        if (pSkin)
            return pSkin;
        pSkin = new SKIN();
        pSkin->load_tdf(filename, scale);
        hash_skin.insert(key, pSkin);
        skins.push_back(pSkin);

        return pSkin;
    }
}
