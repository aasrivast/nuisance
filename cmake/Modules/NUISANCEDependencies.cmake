include(NUISANCEUtils)

include(ROOT)

if(DEFINED ROOT_CXX_STANDARD)
  if(NOT DEFINED CMAKE_CXX_STANDARD OR ROOT_CXX_STANDARD GREATER CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD ${ROOT_CXX_STANDARD})
  endif()
endif()

##########################  Optional Modules  ###############################

add_library(GeneratorCompileDependencies INTERFACE)
add_library(GeneratorLinkDependencies INTERFACE)

DefineEnabledRequiredSwitch(T2KReWeight)
DefineEnabledRequiredSwitch(NIWGLegacy)
DefineEnabledRequiredSwitch(NOvARwgt)
DefineEnabledRequiredSwitch(nusystematics)
DefineEnabledRequiredSwitch(NEUT)
DefineEnabledRequiredSwitch(GENIE)
DefineEnabledRequiredSwitch(NuWro)
DefineEnabledRequiredSwitch(Prob3plusplus)

if (T2KReWeight_ENABLED)
  include(T2KReWeight)

  if(NOT T2KReWeight_FOUND)
    if(T2KReWeight_REQUIRED)
      cmessage(FATAL_ERROR "T2KReWeight was explicitly enabled but cannot be found.")
    endif()
    SET(T2KReWeight_ENABLED FALSE)
  endif()

endif()

if (NIWGLegacy_ENABLED)
  include(FindNIWGLegacy)

  if(NOT NIWGLegacy_FOUND)
    if(NIWGLegacy_REQUIRED)
      cmessage(FATAL_ERROR "NIWGLegacy was explicitly enabled but cannot be found.")
    endif()
    SET(NIWGLegacy_ENABLED FALSE)
  endif()

endif()

if (NOvARwgt_ENABLED)
  include(FindNOvARwgt)

  if(NOT NOvARwgt_FOUND)
    if(NOvARwgt_REQUIRED)
      cmessage(FATAL_ERROR "NOvARwgt was explicitly enabled but cannot be found.")
    endif()
    SET(NOvARwgt_ENABLED FALSE)
  endif()

endif()

if (nusystematics_ENABLED)
  find_package(nusystematics 1.00.3)

  if(NOT nusystematics_FOUND)
    if(nusystematics_REQUIRED)
      cmessage(FATAL_ERROR "nusystematics was explicitly enabled but cannot be found.")
    endif()
    SET(nusystematics_ENABLED FALSE)
  else()
    add_library(NUISANCEnusystematics INTERFACE)
    set_target_properties(NUISANCEnusystematics PROPERTIES 
      INTERFACE_COMPILE_OPTIONS "-Dnusystematics_ENABLED"
      INTERFACE_LINK_LIBRARIES nusystematics::all)

    target_link_libraries(GeneratorCompileDependencies INTERFACE NUISANCEnusystematics)
  endif()
  
endif()

#### Put the generators after the reweighting tools that may depend on them so that
#### libraries are more likely to be in a useful order

#Defines NEUT_ENABLED and NEUT_REQUIRED, 
#if NEUT_ENABLED was passed explicitly 
#then NEUT_REQUIRED is ON, otherwise it is OFF
if (NEUT_ENABLED)
  include(NEUT)

  if(NOT NEUT_FOUND)
    if(NEUT_REQUIRED)
      cmessage(FATAL_ERROR "NEUT was explicitly enabled but cannot be found.")
    endif()
    SET(NEUT_ENABLED FALSE)
  endif()

endif()

if (GENIE_ENABLED)
  find_package(GENIE)

  if(NOT GENIE_FOUND)
    if(GENIE_REQUIRED)
      cmessage(FATAL_ERROR "GENIE was explicitly enabled but cannot be found.")
    endif()
    SET(GENIE_ENABLED FALSE)
  endif()

endif()

if (NuWro_ENABLED)
  find_package(NuWro)

  if(NOT NuWro_FOUND)
    if(NuWro_REQUIRED)
      cmessage(FATAL_ERROR "NuWro was explicitly enabled but cannot be found.")
    endif()
    SET(NuWro_ENABLED FALSE)
  endif()

endif()

if (Prob3plusplus_ENABLED)
  find_package(Prob3plusplus)

  if(NOT Prob3plusplus_FOUND)
    if(Prob3plusplus_REQUIRED)
      cmessage(FATAL_ERROR "Prob3plusplus was explicitly enabled but cannot be found.")
    endif()
    SET(Prob3plusplus_ENABLED FALSE)
  else()
    add_library(NUISANCEProb3plusplus INTERFACE)
    set_target_properties(NUISANCEProb3plusplus PROPERTIES 
      INTERFACE_COMPILE_OPTIONS "-DProb3plusplus_ENABLED"
      INTERFACE_LINK_LIBRARIES Prob3plusplus::all)

    target_link_libraries(GeneratorCompileDependencies INTERFACE NUISANCEProb3plusplus)
  endif()
  
endif()

string(FIND "${CMAKE_SHARED_LINKER_FLAGS}" "-Wl,--no-undefined" NOUNDEF_INDEX)
if(NOUNDEF_INDEX GREATER -1)
  string(REPLACE "-Wl,--no-undefined" "" CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}")
  cmessage(STATUS "Removed -Wl,--no-undefined flag from CMAKE_SHARED_LINKER_FLAGS at the end of NUISANCEDependencies")
endif()