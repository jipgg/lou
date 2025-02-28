print('Hello')


--console:warn("yoo")

lou:get_console():warn("yoo")
lou.console:warn("YOOOOOOOOOO")
lou.console:warn(`{lou}, {lou.console}`)

local _ = lou:get_console()
local c = lou.console
c:warn("im a variable")


-- local con = lou:get_console()
-- con:warn(typeof(lou))
-- con:warn(typeof(con))
--
-- con:warn("yoo")
--
-- lou:get_console()
