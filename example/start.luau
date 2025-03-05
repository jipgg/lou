local shared = require('./shared')
local Object = require('./Objects')

local white = Color(0xff, 0xff, 0xff, 0xff)
local hello_world = lou.texture:from_text(shared.main_font, "Hello, world!", white)

local luau_logo = {
    texture = lou.texture:load_image("resources/Luau_Logo.png"),
    rect = Rect(100, 100, 300, 300),
    angle = 0,
}
function luau_logo:update(delta: number)
end
function luau_logo:render()
    self = self :: typeof(luau_logo)
    self.texture:render(self.rect, self.angle)
end

table.insert(shared.updatables, luau_logo)
table.insert(shared.renderables, luau_logo)

local M = {}


return M
