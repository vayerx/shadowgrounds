
#ifndef UIDEFAULTS_H
#define UIDEFAULTS_H

#include "../ogui/Ogui.h"

//
// This file just gives you some button and select list style objects
// so you don't have to define them yourself everytime...
// 

// NOTICE: presumes that you have only one instance of Ogui
// otherwise you cannot define defaults without attaching them to a 
// specific ogui instance. 

// IF YOU WANT TO HAVE MULTIPLE INSTANCES OF OGUI DO NOT USE THESE!!!

namespace ui
{

  extern OguiButtonStyle *defaultCloseButton;
  extern OguiButtonStyle *defaultUpButton;
  extern OguiButtonStyle *defaultDownButton;
  extern OguiSelectListStyle *defaultSelectList;
  extern IOguiFont *defaultFont; 
  extern IOguiFont *defaultDisabledFont; 
  extern IOguiFont *defaultThinFont; 
  extern IOguiFont *defaultThinWhiteFont; 
  extern IOguiFont *defaultSmallRedFont; 
  extern IOguiFont *defaultRedInfoFont; 
  extern IOguiFont *defaultIngameFont; 
  extern IOguiFont *defaultSmallIngameFont; 
  extern IOguiFont *defaultMediumIngameFont; 
  extern IOguiFont *defaultBigIngameFont; 
  extern IOguiFont *defaultIngameNumbersBoldFont; 
  extern IOguiFont *defaultIngameNumbersBoldSmallFont; 

  void createUIDefaults(Ogui *ogui);

  void deleteUIDefaults();

}

#endif

