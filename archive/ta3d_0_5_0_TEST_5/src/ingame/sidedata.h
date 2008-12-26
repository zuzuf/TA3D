#ifndef __TA3D_INGAME_SideData_H__
# define __TA3D_INGAME_SideData_H__

# include "../stdafx.h"
# include "../ta3dbase.h"


namespace TA3D
{

    struct IntrElementCoords // INT_ELEMENT
    {
        uint16	x1, y1;
        uint16	x2, y2;

    }; // IntrElementCoords


    struct InterfaceData
    {
        //!
        IntrElementCoords EnergyBar;
        //!
        IntrElementCoords EnergyNum;
        //!
        IntrElementCoords EnergyMax;
        //!
        IntrElementCoords Energy0;
        //!
        IntrElementCoords EnergyProduced;
        //!
        IntrElementCoords EnergyConsumed;

        //!
        IntrElementCoords MetalBar;
        //!
        IntrElementCoords MetalNum;
        //!
        IntrElementCoords MetalMax;
        //!
        IntrElementCoords Metal0;
        //!
        IntrElementCoords MetalProduced;
        //!
        IntrElementCoords MetalConsumed;

        //!
        IntrElementCoords UnitName;
        //!
        IntrElementCoords DamageBar;

        //!
        IntrElementCoords UnitName2;
        //!
        IntrElementCoords DamageBar2;

        //!
        IntrElementCoords UnitMetalMake;
        //!
        IntrElementCoords UnitMetalUse;
        //!
        IntrElementCoords UnitEnergyMake;
        //!
        IntrElementCoords UnitEnergyUse;

        //!
        IntrElementCoords Name;
        //!
        IntrElementCoords Description;

        //!
        uint32 metal_color;
        //!
        uint32 energy_color;

    }; // InterfaceData






    /*! \class SideData
    **
    ** \brief
    */
    class SideData
    {
    public:
        SideData();
        ~SideData();

        /*!
        ** \brief
        */
        void init();

        /*!
        ** \brief
        */
        void destroy();

        /*!
        ** \brief
        */
        int	getSideId(const String& side) const;

        /*!
        ** \brief
        */
        void loadData();

    public:
        //!
        int nb_side;
        //! The name
        char* side_name[10];
        //! The prefix
        char* side_pref[10];
        //! The commande
        char* side_com[10];
        //! The GAF interface
        char* side_int[10];	
        //! The interface data (position of the gui elements)
        InterfaceData side_int_data[10];
        //!
        String unit_ext;
        //!
        String unit_dir;
        //!
        String model_dir;
        //!
        String download_dir;
        //!
        String weapon_dir;
        //!
        String guis_dir;
        //!
        String gamedata_dir;

    private:
        /*!
        ** \brief
        ** \param data
        */
        char* get_line(char *data) const;
    };





    /*!
    ** \brief
    */
    IntrElementCoords read_gui_element(UTILS::cTAFileParser* parser, const String& element, bool bottom = false);


    /*!
    **
    */
    extern SideData ta3dSideData;



} // namespace TA3D

#endif // __TA3D_INGAME_SideData_H__
