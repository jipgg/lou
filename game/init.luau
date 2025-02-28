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

local my_rect = Rect(1, 2, 3, 4)
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
