#include "precompiled.h"

#include "CombatSubWindowFactory.h"
#include "AmmoWindow.h"
#include "AmmoWindowCoop.h"
#include "ComboWindow.h"
#include "ElaborateHintMessageWindow.h"
#include "FlashlightWindow.h"
#include "HealthWindow.h"
#include "HealthWindowCoop.h"
#include "UnitHealthBarWindow.h"
#include "UpgradeAvailableWindow.h"
#include "WeaponWindow.h"
#include "WeaponWindowCoop.h"

namespace ui {
    void RegisterGlobalCombatSubwindows() {
        REGISTER_COMBATSUBWINDOW(AmmoWindow);
        REGISTER_COMBATSUBWINDOW(AmmoWindowCoop);
        REGISTER_COMBATSUBWINDOW(ComboWindow);
        REGISTER_COMBATSUBWINDOW(ElaborateHintMessageWindow);
        REGISTER_COMBATSUBWINDOW(FlashlightWindow);
        REGISTER_COMBATSUBWINDOW(HealthWindow);
        REGISTER_COMBATSUBWINDOW(HealthWindowCoop);
        REGISTER_COMBATSUBWINDOW(UnitHealthBarWindow);
        REGISTER_COMBATSUBWINDOW(UpgradeAvailableWindow);
        REGISTER_COMBATSUBWINDOW(WeaponWindow);
        REGISTER_COMBATSUBWINDOW(WeaponWindowCoop);
    }

} // end of namespace ui
