#include <API/ARK/Ark.h>
#include <fstream>
#include <json.hpp>

using json = nlohmann::json;

// 地图解锁顺序配置
static std::vector<std::string> mapOrder;
static std::unordered_map<std::string, std::string> bossTrophyToNextMap;

void LoadConfig()
{
    std::ifstream file(ArkApi::Tools::GetCurrentDir() + "/ArkApi/Plugins/MapUnlocker/config.json");
    if (!file.is_open())
    {
        Log::GetLog()->error("无法打开 config.json");
        return;
    }
    json j;
    file >> j;

    for (auto& entry : j["unlockOrder"])
    {
        std::string bossTrophy = entry["boss_trophy"];
        std::string nextMap = entry["next_map"];
        bossTrophyToNextMap[bossTrophy] = nextMap;
    }
    Log::GetLog()->info("配置加载完成，共 {} 条解锁规则", bossTrophyToNextMap.size());
}

bool PlayerHasTrophy(AShooterPlayerController* player, const std::string& trophyBlueprint)
{
    if (!player) return false;
    auto* inventory = player->GetPlayerInventoryComponent();
    if (!inventory) return false;

    TArray<UPrimalItem*> items = inventory->InventoryItemsField();
    for (UPrimalItem* item : items)
    {
        if (!item) continue;
        FString bp = item->ClassField()->GetName();
        if (bp.ToString().find(trophyBlueprint) != std::string::npos)
            return true;
    }
    return false;
}

void Hook_Teleport(AShooterPlayerController* player, FString* mapName, bool result)
{
    if (!player) return;

    std::string targetMap = mapName->ToString();
    for (auto& pair : bossTrophyToNextMap)
    {
        if (pair.second == targetMap)
        {
            if (!PlayerHasTrophy(player, pair.first))
            {
                ArkApi::GetApiUtils().SendServerMessage(player, FColorList::Red, 
                    "你还没有解锁前置地图BOSS成就，无法进入地图: {}", targetMap);
                *mapName = FString(""); // 阻止传送
            }
        }
    }
}

void Load()
{
    Log::GetLog()->info("MapUnlocker 插件加载中...");
    LoadConfig();
    ArkApi::GetHooks().SetHook("TeleportToMap", &Hook_Teleport);
}

void Unload()
{
    ArkApi::GetHooks().DisableHook("TeleportToMap", &Hook_Teleport);
}

ARK_API void Init()
{
    Load();
}

ARK_API void Shutdown()
{
    Unload();
}
