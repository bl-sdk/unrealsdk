#include "unrealsdk/pch.h"
#include "unrealsdk/unreal/structs/fpropertychangeevent.h"

namespace unrealsdk::unreal {

class UProperty;

FEditPropertyChain::FEditPropertyChain(const std::vector<UProperty*>& chain)
    : size((uint32_t)chain.size()) {
    this->nodes.resize(chain.size());

    size_t idx = 0;
    for (auto prop : chain) {
        this->nodes[idx].value = prop;

        if (idx > 0) {
            this->nodes[idx].prev = &this->nodes[idx - 1];
        }
        if (idx < (chain.size() - 1)) {
            this->nodes[idx].next = &this->nodes[idx + 1];
        }

        idx++;
    }

    if (chain.size() > 0) {
        this->head = &this->nodes.front();
        this->tail = &this->nodes.back();
    }
}

}  // namespace unrealsdk::unreal
