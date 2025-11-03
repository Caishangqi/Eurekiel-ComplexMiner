#include "Game/Gameplay/TreeStamps/AcaciaTreeStamp.hpp"
#include "Engine/Registry/Block/BlockRegistry.hpp"
using namespace enigma::registry::block;

AcaciaTreeStamp::AcaciaTreeStamp(const std::vector<TreeStampBlock>& blocks, const std::string& sizeName)
    : TreeStamp(blocks), m_sizeName(sizeName)
{
    // Initialize Block IDs from BlockRegistry
    InitializeBlockIds(GetLogBlockName(), GetLeavesBlockName());
}

AcaciaTreeStamp AcaciaTreeStamp::CreateSmall()
{
    // Query Block IDs directly from BlockRegistry
    int logId    = BlockRegistry::GetBlockId("simpleminer", "acacia_log");
    int leavesId = BlockRegistry::GetBlockId("simpleminer", "acacia_leaves");

    std::vector<TreeStampBlock> blocks;
    // Trunk (angled)
    blocks.push_back(TreeStampBlock(IntVec3(0, 0, 0), logId));
    blocks.push_back(TreeStampBlock(IntVec3(0, 0, 1), logId));
    blocks.push_back(TreeStampBlock(IntVec3(0, 0, 2), logId));
    blocks.push_back(TreeStampBlock(IntVec3(1, 0, 3), logId));
    blocks.push_back(TreeStampBlock(IntVec3(1, 0, 4), logId));

    // Leaves (Z=4-5, centered on trunk end)
    for (int z = 4; z <= 5; ++z)
    {
        for (int x = 0; x <= 2; ++x)
        {
            for (int y = -1; y <= 1; ++y)
            {
                if (z == 4 && x == 1 && y == 0) continue;
                blocks.push_back(TreeStampBlock(IntVec3(x, y, z), leavesId));
            }
        }
    }
    return AcaciaTreeStamp(blocks, "Small");
}

AcaciaTreeStamp AcaciaTreeStamp::CreateMedium()
{
    // Query Block IDs directly from BlockRegistry
    int logId    = BlockRegistry::GetBlockId("simpleminer", "acacia_log");
    int leavesId = BlockRegistry::GetBlockId("simpleminer", "acacia_leaves");

    std::vector<TreeStampBlock> blocks;
    // Trunk (angled)
    blocks.push_back(TreeStampBlock(IntVec3(0, 0, 0), logId));
    blocks.push_back(TreeStampBlock(IntVec3(0, 0, 1), logId));
    blocks.push_back(TreeStampBlock(IntVec3(0, 0, 2), logId));
    blocks.push_back(TreeStampBlock(IntVec3(1, 0, 3), logId));
    blocks.push_back(TreeStampBlock(IntVec3(1, 0, 4), logId));
    blocks.push_back(TreeStampBlock(IntVec3(1, 0, 5), logId));

    // Leaves (Z=5-6, centered on trunk end)
    for (int z = 5; z <= 6; ++z)
    {
        for (int x = 0; x <= 2; ++x)
        {
            for (int y = -1; y <= 1; ++y)
            {
                if (z == 5 && x == 1 && y == 0) continue;
                blocks.push_back(TreeStampBlock(IntVec3(x, y, z), leavesId));
            }
        }
    }
    return AcaciaTreeStamp(blocks, "Medium");
}

AcaciaTreeStamp AcaciaTreeStamp::CreateLarge()
{
    // Query Block IDs directly from BlockRegistry
    int logId    = BlockRegistry::GetBlockId("simpleminer", "acacia_log");
    int leavesId = BlockRegistry::GetBlockId("simpleminer", "acacia_leaves");

    std::vector<TreeStampBlock> blocks;
    // Trunk (angled)
    blocks.push_back(TreeStampBlock(IntVec3(0, 0, 0), logId));
    blocks.push_back(TreeStampBlock(IntVec3(0, 0, 1), logId));
    blocks.push_back(TreeStampBlock(IntVec3(0, 0, 2), logId));
    blocks.push_back(TreeStampBlock(IntVec3(1, 0, 3), logId));
    blocks.push_back(TreeStampBlock(IntVec3(1, 0, 4), logId));
    blocks.push_back(TreeStampBlock(IntVec3(2, 0, 5), logId));
    blocks.push_back(TreeStampBlock(IntVec3(2, 0, 6), logId));

    // Leaves (Z=6-7, centered on trunk end)
    for (int z = 6; z <= 7; ++z)
    {
        for (int x = 1; x <= 3; ++x)
        {
            for (int y = -1; y <= 1; ++y)
            {
                if (z == 6 && x == 2 && y == 0) continue;
                blocks.push_back(TreeStampBlock(IntVec3(x, y, z), leavesId));
            }
        }
    }
    return AcaciaTreeStamp(blocks, "Large");
}
