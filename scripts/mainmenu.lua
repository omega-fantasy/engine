local screen = UI_screen_container()
local screen_size = UI_screen_size()

local menu_width = 0.5 * screen_size.width
local menu_height = 0.75 * screen_size.height
menu_container = UI_new_container(menu_width, menu_height)
UI_add_child(screen, menu_container, 0.25 * screen_size.width, 0.15 * screen_size.height)

local function switch_to_mapscreen()
    local screen = UI_screen_container()
    UI_screen_clear()
    UI_container_dispose(menu_container)

    local screen_size = UI_screen_size()
    local hud_width = 0.25
    local hud_container = UI_new_hud(hud_width * screen_size.width, screen_size.height)
    local map_container = UI_new_tilemap((1 - hud_width) * screen_size.width, screen_size.height)

    UI_add_child(screen, hud_container, 0, 0)
    UI_add_child(screen, map_container, hud_width * screen_size.width, 0)
end

function new_game_callback(t)
    UI_play_sound("menu2")
    switch_to_mapscreen()
    MAP_randomize()
    return {}
end
local new_button = UI_new_button(menu_width, 0.20 * menu_height, "New Game", "new_game_callback", {})
UI_add_child(menu_container, new_button, 0, 0)

function load_game_callback(t)
    UI_play_sound("menu2")
    Engine_load_state("state.sav")
    switch_to_mapscreen()
    return {}
end
local load_button = UI_new_button(menu_width, 0.20 * menu_height, "Load Game", "load_game_callback", {})
UI_add_child(menu_container, load_button, 0, 0.25 * menu_height)

function options_button_callback(t)
    -- TODO
    return {}
end
local options_button = UI_new_button(menu_width, 0.20 * menu_height, "Options", "options_button_callback", {})
UI_add_child(menu_container, options_button, 0, 0.5 * menu_height)

function exit_button_callback(t)
    os.exit() 
    return {}
end
local exit_button = UI_new_button(menu_width, 0.20 * menu_height, "Exit", "exit_button_callback", {})
UI_add_child(menu_container, exit_button, 0, 0.75 * menu_height)