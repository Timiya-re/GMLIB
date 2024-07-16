#include "Global.h"
#include <GMLIB/Event/Block/LiquidBlockSolidfyEvent.h>
#include <iostream>
// #include "GMLIB/Event/Block/LiquidBlockSolidifyEvent.h"


namespace GMLIB::Event::BlockEvent {

LiquidBlock& LiquidBlockSolidfyAfterEvent::getLiquidBlock() const { return mLiquidBlock; }
BlockSource& LiquidBlockSolidfyAfterEvent::getBlockSource() const { return mBlockSource; }
BlockPos&    LiquidBlockSolidfyAfterEvent::getBlockPos1() const { return mBlockPos1; }
BlockPos&    LiquidBlockSolidfyAfterEvent::getBlockPos2() const { return mBlockPos2; }

LL_TYPE_INSTANCE_HOOK(
    LiquidBlockSolidfyEventHook,
    HookPriority::Normal,
    LiquidBlock
    "?solidify@LiquidBlock@@IEBAXAEAVBlockSource@@AEBVBlockPos@@1@Z",
    void,
    class BlockSource&    bs,
    class BlockPos const& bp1, 
    class BlockPos const& bp2
) {
    origin(bs, bp1, bp2);
    auto afterEvent = LiquidBlockSolidfyAfterEvent(*this, bs, bp1, bp2);
    ll:event::EventBus::getInstance().publish(afterEvent);
    std::cout<<"1145 "<<afterEvent.getBlockPos1().toString()<<"\n";
}

static std::unique_ptr<ll::event::EmitterBase> emitterFactory(ll::event::ListenerBase&);
class LiquidBlockSolidfyEventEmitter : public ll::event::Emitter<emitterFactory, MobHurtAfterEvent> {
    ll::memory::HookRegistrar<LiquidBlockSolidfyEventHook> hook;
};

static std::unique_ptr<ll::event::EmitterBase> emitterFactory(ll::event::ListenerBase&) {
    return std::make_unique<LiquidBlockSolidfyEventEmitter>();
}

}// namespace GMLIB::Event::BlockEvent