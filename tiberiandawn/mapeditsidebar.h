#pragma once

#include <optional>
#include <map>
#include <memory>

#include "dialogcontroldimension.h"
#include "wwkeyboard.h"

class MapEditClass;

class MapEditorSidebar : public DialogControlDimension
{
public:
    struct ObjectCatalogItem
    {
        DialogControlDimension Dimensions;
        const ObjectTypeClass* ObjectType = nullptr;
    };

    struct GridPager
    {
        int ItemCount = -1;
        int ItemsPerPage = -1;
        int PageCount = -1;
        int CurrentPage = -1;
    };

    struct ObjectListPager
    {
        int ItemCount = -1;
        int CurrentIndex = -1;
        int CurrentDepth = -1;
        std::vector<int> PageStartIndexes;
    };
private:
    static constexpr auto ControlMargin = 5;
    static constexpr auto ButtonHeight = 20;
    static constexpr auto FooterHeight = ControlMargin + ButtonHeight + ControlMargin;

    static inline std::vector<ObjectCatalogItem> OverlayCatalog;
    static inline std::vector<ObjectCatalogItem> TerrainCatalog;
    static inline std::vector<ObjectCatalogItem> UnitsCatalog;
    static inline std::vector<ObjectCatalogItem> BuildingsCatalog;

    MapEditClass* Parent = nullptr;
    std::map<int, DialogControlDimension> Dimensions;
    std::map<int, std::unique_ptr<GadgetClass>> Controls;

    // pagination control

    GridPager OverlayGridPager;
    ObjectListPager TerrainPager;
    GridPager UnitsGridPager;
    ObjectListPager BuildingListPager;

    const ObjectTypeClass* CurrentObject = nullptr;
    std::optional<const char*> HelpText;
    int HelpTextX = 0;
    int HelpTextY = 0;

    template<typename T>
    T& Get_Control(const int id)
    {
        return *reinterpret_cast<T*>(Controls[id].get());
    }

    template<int T, typename U>
    U& Get_Control()
    {
        return Get_Control<U>(T);
    }

    void Render_Object_Grid(int control, std::vector<ObjectCatalogItem>& objects, GridPager& pager);
    void Render_Object_List(int control, std::vector<ObjectCatalogItem>& objects, ObjectListPager& pager);
    void Render_Minimap();

    void Init_Controls();
    void Init_Dimensions();
public:

    void Init(MapEditClass* parent);
    void Add_This();
    void Remove_This();

    void Render();
    bool On_Input(const KeyNumType& input, bool forced = false);
};
