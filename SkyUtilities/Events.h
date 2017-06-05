#pragma once

#include <skse/PluginAPI.h>
#include <skse/PapyrusArgs.h>

namespace SKU {
  class IEventHandler
  {
    friend class Plugin;

    protected:
    //
    // Papyrus
    virtual void OnSKSERegisterPapyrusFunctions(VMClassRegistry*)
    {}
  };
}
