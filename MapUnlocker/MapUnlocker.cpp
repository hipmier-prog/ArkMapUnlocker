#include <API/ARK/Ark.h>
#include <json.hpp>
#include <fstream>
#include <unordered_map>
#include <unordered_set>

using json = nlohmann::json;

namespace MapUnlocker
{
    struct MapConfig
    {
        std::string mapName;
        std::vector<std::string> requiredAchievements;
    };

    std::vector<MapConfig> mapOrder;
    std::unordered_map<std::string, MapConfig> mapLookup;

    void LoadConfig()
    {
        std::ifstream file("MapUnlocker/config.json");
        if (!file.is_open())
        {
            Log::GetLog()->error("MapUnlocker: config.json not found!");
            return;
        }

        json data;
        file >> data;

        for (auto& item : data["maps"])
        {
            MapConfig cfg;
            cfg.mapName = item["name"].get<std::string>();
            for (auto& req : item["requires"])
                cfg.requiredAchievements.push_back(req.get<std::string>());

            mapOrder.push_back(cfg);
            mapLookup[cfg.mapName] = cfg;
        }

        Log::GetLog()->info("MapUnlocker: Loaded {} map configs", mapOrder.size());
    }

    bool PlayerHasRequirements(AShooterPlayerController* player, const MapConfig& cfg)
    {
        // 检查成就（替代钥匙避免交易）
        for (const auto& ach : cfg.requiredAchievements)
        {
            if (!player->GetPlayerCharacter() || !player->GetPlayerCharacter()->GetPlayerCharacterAchievementsComponent())
                return false;

            if (!player->GetPlayerCharacter()->GetPlayerCharacterAchievementsComponent()->HasAchievement(FString(ach.c_str())))
                return false;
        }
        return true;
    }

    void OnTravel(APlayerController* player, FString* mapName, bool* result)
    {
        auto it = mapLookup.find(mapName->ToString());
        if (it == mapLookup.end()) return;

        auto shooter = static_cast<AShooterPlayerController*>(player);
        if (!PlayerHasRequirements(shooter, it->second))
        {
            *result = false;
            ArkApi::GetApiUtils().SendServerMessage(shooter, FColorList::Red, "你还没有解锁此地图，请先通关前置地图 BOSS！");
        }
    }

    void CmdStatus(AShooterPlayerController* player, FString*, int)
    {
        ArkApi::GetApiUtils().SendServerMessage(player, FColorList::Green, "=== 地图解锁进度 ===");
        for (auto& cfg : mapOrder)
        {
            bool unlocked = PlayerHasRequirements(player, cfg);
            ArkApi::GetApiUtils().SendServerMessage(
                player,
                unlocked ? FColorList::Green : FColorList::Red,
                "{} : {}",
                cfg.mapName,
                unlocked ? "已解锁" : "未解锁"
            );
        }
    }

    void Load()
    {
        LoadConfig();
        ArkApi::GetHooks().SetHook("AShooterPlayerController.ServerTravel", &OnTravel);
        ArkApi::GetCommands().AddChatCommand("/unlockstatus", &CmdStatus);
        Log::GetLog()->info("MapUnlocker loaded.");
    }

    void Unload()
    {
        ArkApi::GetHooks().DisableHook("AShooterPlayerController.ServerTravel", &OnTravel);
        ArkApi::GetCommands().RemoveChatCommand("/unlockstatus");
        Log::GetLog()->info("MapUnlocker unloaded.");
    }
}

extern "C" __declspec(dllexport) void Plugin_Init()
{
    MapUnlocker::Load();
}

extern "C" __declspec(dllexport) void Plugin_Unload()
{
    MapUnlocker::Unload();
}
