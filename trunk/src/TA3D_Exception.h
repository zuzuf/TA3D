/*  TA3D, a remake of Total Annihilation
    Copyright (C) 2006  Roland BROCHARD

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA*/

/*
**  File: TA3D_Exception.h
** Notes:
**   Cire: This file should be used to handle errors recording.  It is very simple to
**           use.  You can pretty much guaretee that you code is running inside a guard
**           block. and if something don't go the way you want it to just invoke a throw
**           passing a char * or String.
**         When recording guard information use, 'GuardEnter( YOUR_FUNCTION_NAME )', you
**           MUST use GuardLeave() before you exit the function if nothing bad occurs.
**         You can use 'GuardInfo( string )' to record information to the guard log that
**           will enable you to track through to where the error is occuring should it
**           be invoking an exception or some error.
**         If an error occurs it will be logged to error.txt.
**         If you want to handle errors yourself without causing the application to
**           come down you will need to add your own try {} catch() {} block.
*/

#ifndef __TA3D_EXCEPTIONS_H__
# define __TA3D_EXCEPTIONS_H__

# include <string>
# include "stdafx.h"

namespace TA3D
{
namespace Exceptions
{

    /*!
    ** Use this to enter functions.  If you do use this you must use GuardLeave();
    */
    # define GuardEnter(name) \
        static const char *__GUARD__BLOCK__NAME__ = #name; \
        int	__GUARD__UFORMAT__ = get_uformat();	\
        set_uformat( U_UTF8 );	\
        GuardLog(TA3D::format("%sGuarded:%s (%s l.%d)\n", GuardIndentPadding().c_str(), \
                              __GUARD__BLOCK__NAME__, __FILE__, __LINE__) ); \
        set_uformat( __GUARD__UFORMAT__ );	\
        GuardIncrementIndent();


    /*!
    ** if u used GuardLeave(), you will need to use this to leave.
    */
    # define GuardLeave() \
        GuardDecrementIndent(); \
        __GUARD__UFORMAT__ = get_uformat();	\
        set_uformat(U_UTF8);	\
        GuardLog(TA3D::format("%sUnGuard:%s (%s l.%d)\n", GuardIndentPadding().c_str(), \
                                __GUARD__BLOCK__NAME__, __FILE__, __LINE__) );	\
        set_uformat(__GUARD__UFORMAT__);


    /*!
    ** use to record information within your function.
    */
    # define GuardInfo(szStr) \
        GuardLog( TA3D::format("%sINFO:   %s (%s l.%d)\n", GuardIndentPadding().c_str(), \
                               szStr, __FILE__, __LINE__ ) );



    /*!
    ** \warning No function should call this, it is used by the engine itself to start a
    ** guard call.  You can pretty much guarentee that anything in your code
    ** deepter then the engine is being callled as 'guarded'.
    **
    ** Functions should use 'guardenter( name );' when they enter a function, and
    ** can use 'guardinfo( blah );' to recoard data for guard to log should an
    ** error occur, finally if u used 'guardenter' you must call guardleave();
    */
    # define GuardStart(name) \
        { \
            GuardReset(); \
            static const char *__GUARD__BLOCK__NAME__ = #name; \
            GuardLog( TA3D::format( "Guard:%s (%s l.%d)\n", __GUARD__BLOCK__NAME__, __FILE__, __LINE__) ); \
            GuardIncrementIndent(); \
            try \
            {


    /*!
    ** no functions should call this either, it is used by the engine itself to finish
    ** the try catch macro.
    */
    # define GuardCatch() \
            } \
            catch(const char* ErrorName ) \
            { \
                GuardLog( format( "\nString Error thrown\n%s\n", ErrorName ) ); \
                SetExceptionStatus (true); \
            } \
            catch(String &szErr) \
            { \
                GuardLog( format( "\nString Error thrown\n%s\n", szErr.c_str() ) ); \
                SetExceptionStatus (true); \
            } \
            catch (...) \
            { \
                GuardLog(format("\nException caught\n%s\n", GuardGetSysError().c_str())); \
                SetExceptionStatus (true); \
            } \
            GuardDecrementIndent(); \
            GuardLog(format("Unguard: %s\n", __GUARD__BLOCK__NAME__)); \
        }





    //! \name Exception: Prototypes: internal use only.
    //@{

    /*!
    ** \brief
    */
    void GuardIncrementIndent();

    /*!
    ** \brief
    */
    void GuardDecrementIndent();

    /*!
    ** \brief
    */
    void GuardReset();

    /*!
    ** \brief
    */
    void GuardLog(const std::string& szLog);

    /*!
    ** \brief
    */
    const String GuardIndentPadding();

    /*!
    ** \brief
    ** \param e
    */
    void SetExceptionStatus(bool e);

    /*!
    ** \brief
    */
    bool IsExceptionInProgress(void);

    /*!
    ** \brief
    */
    const String GetGuardLog(void);

    /*!
    ** \brief
    */
    String GuardGetSysError();

    /*!
    ** \brief
    */
    void GuardDisplayAndLogError();

    //@}



} // namespace Exceptions
} // namespace TA3D 


#endif // __TA3D_EXCEPTIONS_H__
