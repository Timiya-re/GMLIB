#include "Global.h"
#include <GMLIB/Server/FakeListAPI.h>
#include <mc/network/MinecraftPackets.h>
#include <mc/network/packet/PlayerListEntry.h>
#include <mc/network/packet/PlayerListPacket.h>

namespace GMLIB::Server {

namespace FakeListAPI {

std::unordered_map<std::string, std::string>     mReplaceMap;
std::unordered_map<std::string, PlayerListEntry> mFakeListMap;
bool                                             mSimulatedPlayerOptList = false;

bool EnableHook1 = false;
bool EnableHook2 = false;

} // namespace FakeListAPI

LL_TYPE_INSTANCE_HOOK(
    SendAllFakeListPlayerJoinHook,
    HookPriority::Normal,
    ServerPlayer,
    "?setLocalPlayerAsInitialized@ServerPlayer@@QEAAXXZ",
    bool
) {
    auto pkt    = PlayerListPacket();
    pkt.mAction = PlayerListPacketType::Add;
    for (auto fakeListPair : GMLIB::Server::FakeListAPI::mFakeListMap) {
        pkt.emplace(std::move(fakeListPair.second));
    }
    BinaryStream bs; // DefaultPermission
    bs.writeUnsignedInt64(-1, 0, 0);
    bs.writeUnsignedChar((uchar)1, 0, 0);
    bs.writeUnsignedChar((uchar)CommandPermissionLevel::Any, 0, 0);
    bs.writeUnsignedVarInt(0, 0, 0);
    auto ablitiespkt = MinecraftPackets::createPacket(MinecraftPacketIds::UpdateAbilities);
    ablitiespkt->read(bs);
    pkt.sendToClients();
    ablitiespkt->sendToClients();
    return origin();
}

LL_TYPE_INSTANCE_HOOK(
    FakeListEmplaceHook,
    HookPriority::Normal,
    PlayerListPacket,
    "?emplace@PlayerListPacket@@QEAAX$$QEAVPlayerListEntry@@@Z",
    void,
    PlayerListEntry& entry
) {
    if (this->mAction == PlayerListPacketType::Add) {
        if (GMLIB::Server::FakeListAPI::mSimulatedPlayerOptList) {
            if (ll::service::getLevel()->getPlayer(entry.mId)->isSimulatedPlayer()) {
                return;
            }
        }
        if (GMLIB::Server::FakeListAPI::mReplaceMap.count(entry.mName)) {
            entry.mName = GMLIB::Server::FakeListAPI::mReplaceMap[entry.mName];
        }
    }
    return origin(entry);
}

struct Fakelist_Impl_1 {
    ll::memory::HookRegistrar<SendAllFakeListPlayerJoinHook> r;
};

std::unique_ptr<Fakelist_Impl_1> fakelist_Impl_1;

void enableHook1() {
    if (!fakelist_Impl_1) {
        fakelist_Impl_1 = std::make_unique<Fakelist_Impl_1>();
    }
}

struct Fakelist_Impl_2 {
    ll::memory::HookRegistrar<FakeListEmplaceHook> r;
};

std::unique_ptr<Fakelist_Impl_2> fakelist_Impl_2;

void changeHook2(bool enable) {
    if (enable) {
        if (!fakelist_Impl_2) fakelist_Impl_2 = std::make_unique<Fakelist_Impl_2>();
    } else {
        fakelist_Impl_2.reset();
    }
}

inline void sendAddFakeListPacket(PlayerListEntry entry) {
    auto pkt    = PlayerListPacket();
    pkt.mAction = PlayerListPacketType::Add;
    pkt.emplace(std::move(entry));
    pkt.sendToClients();
}

inline void sendRemoveFakeListPacket(std::vector<PlayerListEntry> entries) {
    auto pkt    = PlayerListPacket();
    pkt.mAction = PlayerListPacketType::Remove;
    for (auto& entry : entries) {
        pkt.emplace(std::move(entry));
    }
    pkt.sendToClients();
}

bool FakeList::addFakeList(PlayerListEntry entry) {
    if (entry.mName.empty()) {
        return false;
    }
    GMLIB::Server::enableHook1();
    GMLIB::Server::FakeListAPI::mFakeListMap.insert({entry.mName, entry});
    sendAddFakeListPacket(entry);
    return true;
}

bool FakeList::addFakeList(std::string name, std::string xuid, ActorUniqueID uniqueId, mce::UUID uuid) {
    if (name.empty()) {
        return false;
    }
    auto entry   = PlayerListEntry(uuid);
    entry.mName  = name;
    entry.mXuid  = xuid;
    entry.mId.id = uniqueId.id;
    GMLIB::Server::enableHook1();
    if (GMLIB::Server::FakeListAPI::mFakeListMap.count(name)) {
        sendRemoveFakeListPacket({GMLIB::Server::FakeListAPI::mFakeListMap[name]});
    }
    GMLIB::Server::FakeListAPI::mFakeListMap.insert({name, entry});
    sendAddFakeListPacket(entry);
    return true;
}

bool FakeList::removeFakeList(std::string nameOrXuid) {
    bool isRemoved = false;
    GMLIB::Server::enableHook1();
    for (auto fakeListPair : GMLIB::Server::FakeListAPI::mFakeListMap) {
        if (fakeListPair.first == nameOrXuid || fakeListPair.second.mXuid == nameOrXuid) {
            GMLIB::Server::FakeListAPI::mFakeListMap.erase(fakeListPair.first);
            sendRemoveFakeListPacket({fakeListPair.second});
            isRemoved = true;
        }
    }
    return isRemoved;
}

void FakeList::removeAllFakeLists() {
    std::vector<PlayerListEntry> entries;
    GMLIB::Server::enableHook1();
    for (auto fakeListPair : GMLIB::Server::FakeListAPI::mFakeListMap) {
        entries.push_back(fakeListPair.second);
    }
    sendRemoveFakeListPacket(entries);
    GMLIB::Server::FakeListAPI::mFakeListMap.clear();
}

PlayerListEntry FakeList::getFakeList(std::string name) { return GMLIB::Server::FakeListAPI::mFakeListMap[name]; }

bool FakeList::checkFakeListExistsName(std::string name) {
    return GMLIB::Server::FakeListAPI::mFakeListMap.count(name);
}

bool FakeList::checkFakeListExists(std::string name, std::string xuid) {
    for (auto fakeListPair : GMLIB::Server::FakeListAPI::mFakeListMap) {
        if (fakeListPair.first == name && fakeListPair.second.mXuid == xuid) {
            return true;
        }
    }
    return false;
}

std::vector<std::string> FakeList::getAllFakeNames() {
    std::vector<std::string> allFakeLists;
    for (auto fakeListPair : GMLIB::Server::FakeListAPI::mFakeListMap) {
        allFakeLists.push_back(fakeListPair.first);
    }
    return allFakeLists;
}

inline void updatePlayerList(std::string realName) {
    ll::service::getLevel()->forEachPlayer([realName](Player& pl) -> bool {
        if (pl.getRealName() == realName) {
            if (pl.isSimulatedPlayer() && !GMLIB::Server::FakeListAPI::mSimulatedPlayerOptList) {
                return true;
            }
            auto entry = PlayerListEntry(pl);
            sendAddFakeListPacket(entry);
        }
        return true;
    });
    if (GMLIB::Server::FakeListAPI::mFakeListMap.count(realName)) {
        sendAddFakeListPacket(GMLIB::Server::FakeListAPI::mFakeListMap[realName]);
    }
}

void FakeList::setListName(std::string realName, std::string fakeName) {
    GMLIB::Server::FakeListAPI::mReplaceMap[realName] = fakeName;
    updatePlayerList(realName);
}

void FakeList::resetListName(std::string realName) {
    GMLIB::Server::FakeListAPI::mReplaceMap.erase(realName);
    updatePlayerList(realName);
}

void FakeList::setSimulatedPlayerListOptimizeEnabled(bool value) {
    GMLIB::Server::FakeListAPI::mSimulatedPlayerOptList = value;
    changeHook2(value);
}

bool FakeList::getSimulatedPlayerListOptimizeEnabled() { return GMLIB::Server::FakeListAPI::mSimulatedPlayerOptList; }

} // namespace GMLIB::Server