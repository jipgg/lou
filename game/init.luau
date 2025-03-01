print('Hello')

--console:warn("yoo")

lou.console:warn("YOOOOOOOOOO")
lou.console:warn(`{lou}, {lou.console}`)

local c = lou.console
c:warn("im a variable")

print(lou.keyboard)


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

local my_rect = Rect(100, 100, 300, 300)
lou:on_render(function()
    lou.renderer:set_draw_color(0xff, 0xff, 0xff)
    lou.renderer:draw_rect(my_rect)
    warn(lou.mouse:position())
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
