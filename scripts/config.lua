local settings = {
    ["resolution"] = { ["width"] = 3000, ["height"] = 1500, ["fullscreen"] = 0 },
    ["mapsize"] = { ["width"] = 1024, ["height"] = 1024 },
    ["tilesize"] = { ["width"] = 16, ["height"] = 16 },
    ["movespeed"] = 20,
    ["infinite_scrolling"] = 1,
    ["use_fast_renderer"] = 1,
    ["keys"] = {
        ["moveup"] = "Up",
        ["movedown"] = "Down",
        ["moveright"] = "Right",
        ["moveleft"] = "Left",
        ["zoomin"] = "WheelUp",
        ["zoomout"] = "WheelDown",
        ["quit"] = "Q"        
    },
    ["colors"] = {
        ["button_topleft"] = 0xFF000096,
        ["button_bottomright"] = 0xFF000096,
        ["button_border"] = 0xFFC8C8C8,
        ["button_selected"] = 0xFFAAAA00,
        ["box_topleft"] = 0xFF000096,
        ["box_bottomright"] = 0xFF000020,
        ["box_border"] = 0xFFC8C8C8
    }
}

set_config("settings", settings)




local buildings = {
    ["startmoney"] = 10000,
    ["max_town_distance"] = 100,
    ["buildings"] = {
        { ["name"] = "inn",  ["price"] = 200, ["properties"] = {} },
        { ["name"] = "shop", ["price"] = 200, ["properties"] = { ["GenerateMoney"] = 50.0 } },
    }
}

set_config("buildings", buildings)






local research = {
    ["items"] = {
        { ["name"] = "Research1", ["cost"] = 100, ["max_progress"] = 10, ["description"] = "This is a description for Research1." },
        { ["name"] = "Research2", ["cost"] = 10,  ["max_progress"] = 5,  ["description"] = "This is a description for Research2." },
    }
}

set_config("research", research)





local generate_textures = {
-- Grounds
    { ["name"] = "water", ["type"] = "noise", ["color"] = 0xFF0048C8, ["variance"] = 0.04 },
    { ["name"] = "grass", ["type"] = "noise", ["color"] = 0xFF438D17, ["variance"] = 0.03 },
    { ["name"] = "sand",  ["type"] = "noise", ["color"] = 0xFFDCC47E, ["variance"] = 0.02 },
    { ["name"] = "sand2", ["type"] = "noise", ["color"] = 0xFFC29858, ["variance"] = 0.02 },
    { ["name"] = "earth", ["type"] = "noise", ["color"] = 0xFF875C25, ["variance"] = 0.02 },
    { ["name"] = "earth2",["type"] = "noise", ["color"] = 0xFF9B7039, ["variance"] = 0.03 },
    { ["name"] = "snow",  ["type"] = "noise", ["color"] = 0xFFB4B4DC, ["variance"] = 0.01 },
    { ["name"] = "mountain", ["type"] = "noise", ["color"] = 0xFFAF7F40, ["variance"] = 0.03 },
    { ["name"] = "mountain_wall", ["type"] = "noise", ["color"] = 0xFF684020, ["variance"] = 0.1 },
-- Building Parts
    { ["name"] = "facade1", ["type"] = "tiled", ["color"] = 0xFF655C73, ["variance"] = 0.04, ["offset"] = 0.5, ["tiles_vertical"] = 4, ["tiles_horizontal"] = 2 },
    { ["name"] = "roof1",   ["type"] = "tiled", ["color"] = 0xFF9B2828, ["variance"] = 0.02, ["offset"] = 0.5, ["tiles_vertical"] = 2, ["tiles_horizontal"] = 4 },
    { ["name"] = "roof2",   ["type"] = "tiled", ["color"] = 0xFF28289B, ["variance"] = 0.02, ["offset"] = 0.5, ["tiles_vertical"] = 2, ["tiles_horizontal"] = 4 },
    { ["name"] = "door",    ["type"] = "tiled", ["color"] = 0xFF775028, ["variance"] = 0.05, ["offset"] = 0,   ["tiles_vertical"] = 1, ["tiles_horizontal"] = 5 },
-- Buildings
    { ["name"] = "inn",  ["type"] = "building", ["width"] = 5, ["height"] = 2, ["depth"] = 3, ["facade"] = "facade1", ["roof"] = "roof1", ["objects"] = { {["name"] = "door", ["x"] = 3, ["y"] = 4} } },
    { ["name"] = "shop", ["type"] = "building", ["width"] = 3, ["height"] = 3, ["depth"] = 2, ["facade"] = "facade1", ["roof"] = "roof2", ["objects"] = { {["name"] = "door", ["x"] = 1, ["y"] = 4} } },
    { ["name"] = "town", ["type"] = "building", ["width"] = 1, ["height"] = 1, ["depth"] = 1, ["facade"] = "facade1", ["roof"] = "roof1", ["objects"] = {} },
-- Plants
    { ["name"] = "bush1", ["type"] = "plant", ["width"] = 1, ["height"] = 1, ["color_crown"] = 0xFF8CB43C, ["color_trunk"] = 0xFF8CB43C, ["variance"] = 0.06, ["width_crown"] = 0.9, ["height_crown"] = 0.7, ["width_trunk"] = 0.0, ["height_trunk"] = 0.0 },
    { ["name"] = "bush2", ["type"] = "plant", ["width"] = 1, ["height"] = 1, ["color_crown"] = 0xFF8CB43C, ["color_trunk"] = 0xFF8CB43C, ["variance"] = 0.06, ["width_crown"] = 0.8, ["height_crown"] = 0.6, ["width_trunk"] = 0.0, ["height_trunk"] = 0.0 },
    { ["name"] = "bush3", ["type"] = "plant", ["width"] = 1, ["height"] = 1, ["color_crown"] = 0xFF8CB43C, ["color_trunk"] = 0xFF8CB43C, ["variance"] = 0.06, ["width_crown"] = 1.0, ["height_crown"] = 0.9, ["width_trunk"] = 0.0, ["height_trunk"] = 0.0 },
    { ["name"] = "tree_small", ["type"] = "plant", ["width"] = 1, ["height"] = 2, ["color_crown"] = 0xFF8CB43C, ["color_trunk"] = 0xFFAF7F40, ["variance"] = 0.06, ["width_crown"] = 1.0, ["height_crown"] = 0.6, ["width_trunk"] = 0.7, ["height_trunk"] = 0.2 },
    { ["name"] = "tree_big",   ["type"] = "plant", ["width"] = 2, ["height"] = 2, ["color_crown"] = 0xFF8CB43C, ["color_trunk"] = 0xFFAF7F40, ["variance"] = 0.06, ["width_crown"] = 0.7, ["height_crown"] = 0.75, ["width_trunk"] = 0.4, ["height_trunk"] = 0.25 },
}

set_config("textures", generate_textures)
    




local mapgen = {
    ["sample_factor"] = 3, ["sample_distance"] = 6, ["num_cells"] = 16,
    ["elevations"] = {
    {
        ["quantity"] = 0.57, ["blocking"] = 1, ["blend"] = 0,
        ["biomes"] = { {["name"] = "water", ["temperature_max"] = 100, 
            ["vegetation"] = {} 
        }}
    },
    {
        ["quantity"] = 0.0175, ["blocking"] = 0, ["blend"] = 1,
        ["biomes"] = { {["name"] = "sand2", ["temperature_max"] = 100, 
            ["vegetation"] = {} 
        }}
    },
    {
        ["quantity"] = 0.11, ["blocking"] = 0, ["blend"] = 1,
        ["biomes"] = { 
            { ["name"] = "snow", ["temperature_max"] = 30, 
                ["vegetation"] = {} 
            },
            { ["name"] = "grass", ["temperature_max"] = 70, 
                ["vegetation"] = {
                    { ["name"] = "tree_small", ["quantity"] = 0.15 },
                    { ["name"] = "tree_big", ["quantity"] = 0.01 },
                    { ["name"] = "bush1", ["quantity"] = 0.01 },
                    { ["name"] = "bush2", ["quantity"] = 0.01 },
                    { ["name"] = "bush3", ["quantity"] = 0.01 }
                } 
            },
            { ["name"] = "sand", ["temperature_max"] = 100, 
                ["vegetation"] = {} 
            },
        }
    },
    {
        ["quantity"] = 0.01, ["blocking"] = 0, ["blend"] = 1,
        ["biomes"] = { {["name"] = "earth2", ["temperature_max"] = 100, 
            ["vegetation"] = {} 
        }} 
    },
    {
        ["quantity"] = 0.2925, ["blocking"] = 0, ["blend"] = 0,
        ["biomes"] = { {["name"] = "earth2", ["temperature_max"] = 100, 
            ["name_wall"] = "mountain_wall", ["max_height"] = 32, ["wall_height"] = 2, ["vegetation"] = {} 
        }} 
    }
    }
}

set_config("mapgen", mapgen)
