print('Hello')

local main_font = Font("resources/main.ttf", 100)
local white = Color(0xff, 0xff, 0xff, 0xff)
local hello_world = lou.texture:from_text(main_font, "Hello, world!", white)

--console:warn("yoo")

lou.console:warn("YOOOOOOOOOO")
lou.console:warn(`{lou}, {lou.console}`)

local c = lou.console
c:warn("im a variable")

print(lou.keyboard)

print(Color(1, 2, 3, 4):as_tuple())

lou.keyboard:released(function(key: string)
    print(`key is '{key}'`)
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
local my_rect = Rect(100, 100, 300, 300)
local my_other_rect = Rect(300, 100, 300, 400)
local color = Color(0x0, 0, 0x77)
lou:on_update(function(delta_seconds: number)
    angle += 2 * delta_seconds
    my_other_rect.x = 300 + 200 * math.sin(angle)
    my_other_rect.y = 300 + 200 * math.sin(angle * .9)
    my_other_rect.w = 300 - 300 * math.cos(angle * .9)
    my_other_rect.h = 300 + 300 * math.sin(angle * 1.1)
end)
lou:on_render(function()
    lou.renderer:set_draw_color(0xff, 0xff, 0xff)
    lou.renderer:draw_rect(my_rect)
    lou.renderer:set_draw_color(color:as_tuple())
    lou.renderer:fill_rect(my_other_rect)
    --lou.renderer:render_texture(hello_world, Rect(100, 100, 100, 100))
    lou.renderer:render_texture(hello_world, 200, 100)
end)
lou.console:error(my_rect.x, my_rect.y)
lou.console:print(my_rect)
lou.console:print(Color(0xff, 0xff, 0xff, 0xff))
lou.console:print(Point(2, 1))

-- local con = lou:get_console()
-- con:warn(typeof(lou))
-- con:warn(typeof(con))
--
-- con:warn("yoo")
--
-- lou:get_console()
