#pragma once

#include <skse/PapyrusVM.h>
#include <skse/PapyrusObjects.h>
#include <skse/PapyrusEvents.h>

#include <memory>
#include <vector>
#include <any>

namespace SKU {
  class PapyrusEventHandler;

  class PapyrusEvent : public IFunctionArguments
  {
    friend class PapyrusEventHandler;
    friend class std::unique_ptr<PapyrusEvent>;

    public:
    using Args = std::vector< std::any >;

    public:
    PapyrusEvent(Args &&args) noexcept;
    ~PapyrusEvent();

    // SKSE internal
    public:
    bool Copy(Output * dst);

    private:
    std::vector< std::any > arguments;
  };
}
