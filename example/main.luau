print('Hello')
local shared = require('./shared')

local white = rgb(0xff, 0xff, 0xff)
print(lou.texture)
local hello_world = lou.texture:from_text(shared.main_font, "Hello, world!", vec3(white))
local luau_logo = lou.texture:load_image("resources/Luau_Logo.png")

print(lou.console)
lou.console:warn("YOOOOOOOOOO")
lou.console:warn(`{lou}, {lou.console}`)

warn(totuple(white))

print(lou.keyboard)
local cbhandle: Lou_Callback_Handle
cbhandle = lou.keyboard:released(function(key: string)
    print(`key is '{key}'`)
    if key == 'T' then
        print(`destroying handle {cbhandle}`)
        cbhandle:destroy()
    end
end)

lou.window:set_title("YOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO")
warn(lou.window:title())

lou.keyboard:released(function(key: string)
    if key == 'R' then
        lou.window:maximize()
    elseif key == 'T' then
        lou.window:minimize()
    elseif key == 'E' then
        lou.window:restore()
    end
end)

local angle = 0
local my_rect = rect(100, 100, 300, 300)
local my_other_rect = rect(300, 100, 300, 400)
local color = rgb(0x0, 0, 0x77)

lou:on_update(function(delta_seconds: number)
    angle += 2 * delta_seconds
    my_other_rect = rect(
        300 + 200 * math.sin(angle),
        300 + 200 * math.sin(angle * .9),
        300 - 300 * math.cos(angle * .9),
        300 + 300 * math.sin(angle * 1.1)
    )
end)
lou:on_render(function()
    for i, v in shared.renderables do
        v:render()
    end
    lou.renderer:set_draw_color(rgb(0xff, 0xff, 0xff))
    lou.renderer:draw_rect(my_rect)
    lou.renderer:set_draw_color(color)
    lou.renderer:fill_rect(my_other_rect)
    --lou.renderer:render_texture(hello_world, Rect(100, 100, 100, 100))
    local size = hello_world.size
    print(hello_world.color)
    hello_world:render(rect(100, 100, size.X, size.Y), 0)
    --lou.renderer:render_texture(luau_logo, Rect(1200, 100, 200, 200))
    luau_logo:render(rect(100, 100, 200, 200), math.deg(angle))
end)
lou.console:print(my_rect)
lou.console:print(vector.create(0xff, 0xff, 0xff))

-- local con = lou:get_console()
-- con:warn(typeof(lou))
-- con:warn(typeof(con))
--
-- con:warn("yoo")
--
-- lou:get_console()
