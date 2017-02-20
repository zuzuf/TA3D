#include "table.h"
#include "i18n.h"


namespace TA3D
{

    QString TranslationTable::gameTime;
    QString TranslationTable::units;
    QString TranslationTable::speed;



	void TranslationTable::Update()
	{
		gameTime = I18N::Translate("game time");
		units = I18N::Translate("units");
		speed = I18N::Translate("speed");
	}


} // namespace TA3D
