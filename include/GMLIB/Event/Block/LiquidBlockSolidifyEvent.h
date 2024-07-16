#pragma once
#include "GMLIB/Macros.h"
#include "mc/world/level/block/LiquidBlock.h"
#include "ll/api/event/Cancellable.h"


namespace GMLIB::Event::BlockEvent {

class LiquidBlockSolidfyAfterEvent final : public ll::event::Cancellable<ll::event::Event> {
protected:
    LiquidBlock& mLiquidBlock;
    BlockSource& mBlockSource;
    BlockPos&    mBlockPos1;
    BlockPos&    mBlockPos2;

public:
    constexpr explicit LiquidBlockSolidfyAfterEvent(
        LiquidBlock&    liquidBlock,
        BlockSource&    blockSource,
        BlockPos&       blockPos1,
        BlockPos&       blockPos2
    ) : Cancellable(),
        mLiquidBlock(liquidBlock),
        mBlockSource(blockSource),
        mBlockPos1(blockPos1),
        mBlockPos2(blockPos2) {}


    GMLIB_API LiquidBlock& getLiquidBlock() const;
    GMLIB_API BlockSource& getBlockSource() const;
    GMLIB_API BlockPos&    getBlockPos1() const;
    GMLIB_API BlockPos&    getBlockPos2() const;

};

} //namespace GMLIB::Event::BlockEvent
