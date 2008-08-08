
#
# AutoTest, if enabled
#
IF(${TA3D_REGRESSION_TEST} EQUAL 1)
    
    # Strings
    ADD_EXECUTABLE(regression-string  string.cpp)
    target_link_libraries(regression-string stdafx logs threads misc)

    # TDF Parser
    ADD_EXECUTABLE(regression-tdfparser  tdf.cpp)
    target_link_libraries(regression-tdfparser stdafx tdf logs threads misc)

ENDIF(${TA3D_REGRESSION_TEST} EQUAL 1)

