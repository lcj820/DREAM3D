SET (DREAM3D_OIMImport_HDRS
  ${DREAM3DLib_SOURCE_DIR}/OIMImport/OIMImport.h
)

SET (DREAM3D_OIMImport_SRCS
 ${DREAM3DLib_SOURCE_DIR}/OIMImport/OIMImport.cpp
)
cmp_IDE_SOURCE_PROPERTIES( "DREAM3DLib/OIMImport" "${DREAM3D_OIMImport_HDRS}" "${DREAM3D_OIMImport_SRCS}" "0")
if ( ${DREAM3D_INSTALL_FILES} EQUAL 1 )
    INSTALL (FILES ${DREAM3D_OIMImport_HDRS}
            DESTINATION include/DREAM3D/OIMImport
            COMPONENT Headers   )
endif()
