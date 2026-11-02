#include "addonmanager.h"
#include "usbhostmanager.h"
#include "storagemanager.h"

bool AddonManager::LoadAddon(GPAddon* addon) {
    if (addon->available()) {
        AddonBlock * block = new AddonBlock;
        addon->setup();
        block->ptr = addon;
        addons.push_back(block);
        return true;
    } else {
        delete addon; // Don't use the memory if we don't have to   
    }

    return false;
}

bool AddonManager::LoadUSBAddon(GPAddon* addon) {
    bool ret = LoadAddon(addon);
    if ( ret == true )
        USBHostManager::getInstance().pushListener(addon->getListener());
    return ret;
}

bool AddonManager::LoadInputAddon(GPAddon* addon) {
    bool ret = LoadAddon(addon);
    if ( ret == true )
        inputAddons.push_back(addon);
    return ret;
}

void AddonManager::ReinitializeAddons() {
    // Loop through all addons and process any that match our type
    for (std::vector<AddonBlock*>::iterator it = addons.begin(); it != addons.end(); it++) {
        (*it)->ptr->reinit();
    }
}

void AddonManager::ReadInputAddons() {
    // Nothing can hold an external action without an input add-on
    if (inputAddons.empty()) return;

    Storage::getInstance().GetGamepad()->clearExternalActionsHeld();
    for (GPAddon* addon : inputAddons) {
        addon->readInputs();
    }
}

void AddonManager::PreprocessAddons() {
    // Loop through all addons and process any that match our type
    for (std::vector<AddonBlock*>::iterator it = addons.begin(); it != addons.end(); it++) {
        (*it)->ptr->preprocess();
    }
}

void AddonManager::ProcessAddons() {
    // Loop through all addons and process any that match our type
    for (std::vector<AddonBlock*>::iterator it = addons.begin(); it != addons.end(); it++) {
        (*it)->ptr->process();
    }
}

void AddonManager::PostprocessAddons(bool reportSent) {
    // Loop through all addons and process any that match our type
    for (std::vector<AddonBlock*>::iterator it = addons.begin(); it != addons.end(); it++) {
        (*it)->ptr->postprocess(reportSent);
    }
}

// HACK : change this for NeoPicoLED
GPAddon * AddonManager::GetAddon(std::string name) { // hack for NeoPicoLED
    for (std::vector<AddonBlock*>::iterator it = addons.begin(); it != addons.end(); it++) {
        if ( (*it)->ptr->name() == name )
            return (*it)->ptr;
    }
    return nullptr;
}
